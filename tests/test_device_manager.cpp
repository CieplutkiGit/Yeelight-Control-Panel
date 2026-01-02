#include "../src/core/device/DeviceManager.h"

#include <QSignalSpy>
#include <QtTest>

class DeviceManagerTest final : public QObject {
    Q_OBJECT

private slots:
    void addsAndValidatesManualDevices();
    void removesRememberedDevices();
};

void DeviceManagerTest::addsAndValidatesManualDevices() {
    DeviceManager manager;
    QSignalSpy added(&manager, &DeviceManager::deviceAdded);
    QVERIFY(manager.addManualDevice(QStringLiteral("127.0.0.1"), 55443));
    QCOMPARE(added.size(), 1);
    QCOMPARE(manager.devices().size(), 1);
    QCOMPARE(manager.devices().first()->info().stableId(), QStringLiteral("127.0.0.1:55443"));
    QVERIFY(!manager.addManualDevice(QStringLiteral("example.invalid"), 55443));
}

void DeviceManagerTest::removesRememberedDevices() {
    DeviceManager manager;
    QVERIFY(manager.addManualDevice(QStringLiteral("127.0.0.1"), 55443));
    manager.removeRememberedDevice(QStringLiteral("127.0.0.1:55443"));
    QVERIFY(manager.devices().isEmpty());
}

QTEST_MAIN(DeviceManagerTest)
#include "test_device_manager.moc"

