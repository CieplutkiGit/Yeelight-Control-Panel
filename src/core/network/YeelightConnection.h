#pragma once

#include "../model/DeviceInfo.h"
#include "../protocol/YeelightMessageParser.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class YeelightConnection final : public QObject {
    Q_OBJECT

public:
    enum class Status {
        Disconnected,
        Connecting,
        Connected,
        Reconnecting,
        Error
    };
    Q_ENUM(Status)

    explicit YeelightConnection(const DeviceInfo& device, QObject* parent = nullptr);

    void connectToDevice();
    void disconnectFromDevice();
    void send(const QJsonObject& command);
    [[nodiscard]] Status status() const;
    [[nodiscard]] DeviceInfo deviceInfo() const;
    [[nodiscard]] int queuedCommandCount() const;

signals:
    void statusChanged(Status status);
    void responseReceived(int requestId, const QJsonArray& result);
    void commandFailed(int requestId, int code, const QString& message);
    void propertiesChanged(const QVariantMap& properties);
    void protocolError(const QString& message);

private slots:
    void handleConnected();
    void handleDisconnected();
    void handleReadyRead();
    void handleSocketError(QAbstractSocket::SocketError error);

private:
    struct QueuedCommand {
        QJsonObject command;
        QString method;
        bool critical = false;
    };
    struct PendingRequest {
        QString method;
        qint64 sentAtMs = 0;
    };

    static bool isCritical(const QString& method);
    static bool isCoalescible(const QString& method);
    void setStatus(Status status);
    void queueCommand(const QJsonObject& command);
    void writeCommand(const QJsonObject& command);
    void flushQueue();
    void scheduleReconnect();
    void completeRequest(int requestId);

    DeviceInfo device_;
    QTcpSocket socket_;
    YeelightMessageParser parser_;
    QList<QueuedCommand> queue_;
    QHash<int, PendingRequest> pending_;
    QTimer reconnectTimer_;
    Status status_ = Status::Disconnected;
    bool explicitlyDisconnected_ = true;
    int reconnectAttempt_ = 0;
    int nextRequestId_ = 1;
};

