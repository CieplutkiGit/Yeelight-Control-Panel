#include "YeelightConnection.h"

#include "../protocol/YeelightCommand.h"

#include <QDateTime>
#include <QLoggingCategory>

#include <algorithm>

Q_LOGGING_CATEGORY(connectionLog, "connection")

namespace {
constexpr int MaximumQueuedCommands = 100;
constexpr int CommandTimeoutMs = 3000;
const QList<int> ReconnectDelaysMs{1000, 2000, 4000, 8000, 15000, 30000};
}

YeelightConnection::YeelightConnection(const DeviceInfo& device, QObject* parent)
    : QObject(parent)
    , device_(device)
    , parser_(this) {
    reconnectTimer_.setSingleShot(true);
    connect(&reconnectTimer_, &QTimer::timeout, this, [this] {
        if (!explicitlyDisconnected_) {
            setStatus(Status::Reconnecting);
            socket_.connectToHost(device_.ipAddress, device_.port);
        }
    });
    connect(&socket_, &QTcpSocket::connected, this, &YeelightConnection::handleConnected);
    connect(&socket_, &QTcpSocket::disconnected, this, &YeelightConnection::handleDisconnected);
    connect(&socket_, &QTcpSocket::readyRead, this, &YeelightConnection::handleReadyRead);
    connect(&socket_, &QTcpSocket::errorOccurred, this, &YeelightConnection::handleSocketError);
    connect(&socket_, &QTcpSocket::bytesWritten, this, [](qint64) {});

    connect(&parser_, &YeelightMessageParser::responseReceived, this,
        [this](int id, const QJsonArray& result) {
            completeRequest(id);
            emit responseReceived(id, result);
        });
    connect(&parser_, &YeelightMessageParser::commandFailed, this,
        [this](int id, int code, const QString& message) {
            completeRequest(id);
            emit commandFailed(id, code, message);
        });
    connect(&parser_, &YeelightMessageParser::propertiesChanged,
        this, &YeelightConnection::propertiesChanged);
    connect(&parser_, &YeelightMessageParser::protocolError,
        this, &YeelightConnection::protocolError);
}

void YeelightConnection::connectToDevice() {
    explicitlyDisconnected_ = false;
    reconnectTimer_.stop();
    if (socket_.state() == QAbstractSocket::ConnectedState
        || socket_.state() == QAbstractSocket::ConnectingState) {
        return;
    }
    setStatus(Status::Connecting);
    socket_.connectToHost(device_.ipAddress, device_.port);
}

void YeelightConnection::disconnectFromDevice() {
    explicitlyDisconnected_ = true;
    reconnectTimer_.stop();
    queue_.clear();
    pending_.clear();
    parser_.reset();
    socket_.disconnectFromHost();
    if (socket_.state() == QAbstractSocket::UnconnectedState) {
        setStatus(Status::Disconnected);
    }
}

void YeelightConnection::send(const QJsonObject& command) {
    const int id = command.value(QStringLiteral("id")).toInt(-1);
    const QString method = command.value(QStringLiteral("method")).toString();
    if (id <= 0 || method.isEmpty()) {
        emit protocolError(QStringLiteral("Cannot send a command without a valid ID and method."));
        return;
    }
    nextRequestId_ = qMax(nextRequestId_, id + 1);
    if (socket_.state() == QAbstractSocket::ConnectedState) {
        writeCommand(command);
        return;
    }
    queueCommand(command);
    connectToDevice();
}

YeelightConnection::Status YeelightConnection::status() const {
    return status_;
}

DeviceInfo YeelightConnection::deviceInfo() const {
    return device_;
}

int YeelightConnection::queuedCommandCount() const {
    return static_cast<int>(queue_.size());
}

bool YeelightConnection::isCritical(const QString& method) {
    return method == QStringLiteral("set_power") || method == QStringLiteral("toggle");
}

bool YeelightConnection::isCoalescible(const QString& method) {
    return method == QStringLiteral("set_bright")
        || method == QStringLiteral("set_rgb")
        || method == QStringLiteral("set_hsv")
        || method == QStringLiteral("set_ct_abx");
}

void YeelightConnection::setStatus(Status status) {
    if (status_ == status) {
        return;
    }
    status_ = status;
    emit statusChanged(status_);
}

void YeelightConnection::queueCommand(const QJsonObject& command) {
    const QString method = command.value(QStringLiteral("method")).toString();
    if (isCoalescible(method)) {
        for (qsizetype index = queue_.size() - 1; index >= 0; --index) {
            if (queue_.at(index).method == method) {
                queue_[index].command = command;
                return;
            }
        }
    }

    if (static_cast<int>(queue_.size()) >= MaximumQueuedCommands) {
        auto removable = std::find_if(queue_.begin(), queue_.end(),
            [](const QueuedCommand& item) { return !item.critical; });
        if (removable != queue_.end()) {
            queue_.erase(removable);
        } else if (!isCritical(method)) {
            emit protocolError(QStringLiteral("Command queue is full."));
            return;
        }
    }
    queue_.append({command, method, isCritical(method)});
}

void YeelightConnection::writeCommand(const QJsonObject& command) {
    const int id = command.value(QStringLiteral("id")).toInt();
    const QString method = command.value(QStringLiteral("method")).toString();
    socket_.write(YeelightCommand::serialize(command));
    pending_.insert(id, {method, QDateTime::currentMSecsSinceEpoch()});
    QTimer::singleShot(CommandTimeoutMs, this, [this, id] {
        if (!pending_.contains(id)) {
            return;
        }
        pending_.remove(id);
        emit commandFailed(id, -1, QStringLiteral("Command timed out."));
    });
}

void YeelightConnection::flushQueue() {
    const auto commands = queue_;
    queue_.clear();
    for (const auto& command : commands) {
        writeCommand(command.command);
    }
}

void YeelightConnection::handleConnected() {
    reconnectAttempt_ = 0;
    setStatus(Status::Connected);
    flushQueue();
    const auto initialState = YeelightCommand::getProperties(nextRequestId_++, {
        QStringLiteral("power"), QStringLiteral("bright"), QStringLiteral("ct"),
        QStringLiteral("rgb"), QStringLiteral("hue"), QStringLiteral("sat"),
        QStringLiteral("color_mode"), QStringLiteral("flowing"),
        QStringLiteral("delayoff"), QStringLiteral("music_on"),
        QStringLiteral("name")
    });
    if (initialState.success) {
        writeCommand(initialState.command);
    }
}

void YeelightConnection::handleDisconnected() {
    parser_.reset();
    pending_.clear();
    if (explicitlyDisconnected_) {
        setStatus(Status::Disconnected);
        return;
    }
    scheduleReconnect();
}

void YeelightConnection::handleReadyRead() {
    parser_.feed(socket_.readAll());
}

void YeelightConnection::handleSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    qCWarning(connectionLog) << "Yeelight connection error:" << socket_.errorString();
    setStatus(Status::Error);
    if (!explicitlyDisconnected_ && socket_.state() == QAbstractSocket::UnconnectedState) {
        scheduleReconnect();
    }
}

void YeelightConnection::scheduleReconnect() {
    if (explicitlyDisconnected_ || reconnectTimer_.isActive()) {
        return;
    }
    const int index = qMin(
        reconnectAttempt_,
        static_cast<int>(ReconnectDelaysMs.size()) - 1
    );
    reconnectTimer_.start(ReconnectDelaysMs.at(index));
    ++reconnectAttempt_;
    setStatus(Status::Reconnecting);
}

void YeelightConnection::completeRequest(int requestId) {
    pending_.remove(requestId);
}
