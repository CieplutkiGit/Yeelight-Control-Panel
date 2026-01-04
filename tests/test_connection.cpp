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
    connection.send(YeelightCommand::toggle(1).command);
    QTRY_VERIFY_WITH_TIMEOUT(
        server.received().contains("\"method\":\"toggle\""),
        1000
    );
    QTRY_VERIFY_WITH_TIMEOUT(
        server.received().contains("\"method\":\"get_prop\""),
        1000
    );
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

QTEST_GUILESS_MAIN(ConnectionTest)
#include "test_connection.moc"
