#include "../src/core/network/YeelightConnection.h"
#include "../src/core/protocol/YeelightCommand.h"
#include "mocks/MockYeelightServer.h"

#include <QSignalSpy>
#include <QtTest>

class ConnectionTest final : public QObject {
    Q_OBJECT

private slots:
    void sendsQueuedCommandAfterConnecting();
    void coalescesContinuousControls();
};

void ConnectionTest::sendsQueuedCommandAfterConnecting() {
    MockYeelightServer server;
    QVERIFY(server.listen());
    DeviceInfo device;
    device.ipAddress = QStringLiteral("127.0.0.1");
    device.port = server.port();
    YeelightConnection connection(device);
    QSignalSpy received(&server, &MockYeelightServer::commandReceived);
    connection.send(YeelightCommand::toggle(1).command);
    QVERIFY(received.wait(1000));
    QVERIFY(server.received().contains("\"method\":\"toggle\""));
    QVERIFY(server.received().contains("\"method\":\"get_prop\""));
}

void ConnectionTest::coalescesContinuousControls() {
    DeviceInfo device;
    device.ipAddress = QStringLiteral("127.0.0.1");
    device.port = 1;
    YeelightConnection connection(device);
    connection.send(YeelightCommand::setBrightness(1, 10, 300).command);
    connection.send(YeelightCommand::setBrightness(2, 20, 300).command);
    QCOMPARE(connection.queuedCommandCount(), 1);
    connection.disconnectFromDevice();
}

QTEST_MAIN(ConnectionTest)
#include "test_connection.moc"

