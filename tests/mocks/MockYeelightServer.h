#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class MockYeelightServer final : public QObject {
    Q_OBJECT

public:
    explicit MockYeelightServer(QObject* parent = nullptr);
    bool listen();
    quint16 port() const;
    QByteArray received() const;
    void sendLine(const QByteArray& json);

signals:
    void commandReceived();

private:
    QTcpServer server_;
    QTcpSocket* client_ = nullptr;
    QByteArray received_;
};

