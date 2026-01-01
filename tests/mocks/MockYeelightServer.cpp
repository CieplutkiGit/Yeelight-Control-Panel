#include "MockYeelightServer.h"

MockYeelightServer::MockYeelightServer(QObject* parent)
    : QObject(parent) {
    connect(&server_, &QTcpServer::newConnection, this, [this] {
        client_ = server_.nextPendingConnection();
        connect(client_, &QTcpSocket::readyRead, this, [this] {
            received_.append(client_->readAll());
            emit commandReceived();
        });
    });
}

bool MockYeelightServer::listen() {
    return server_.listen(QHostAddress::LocalHost, 0);
}

quint16 MockYeelightServer::port() const {
    return server_.serverPort();
}

QByteArray MockYeelightServer::received() const {
    return received_;
}

void MockYeelightServer::sendLine(const QByteArray& json) {
    if (client_ != nullptr) {
        client_->write(json + "\r\n");
    }
}

