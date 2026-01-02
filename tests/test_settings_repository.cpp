#include "../src/core/persistence/SettingsRepository.h"

#include <QTemporaryDir>
#include <QtTest>

class SettingsRepositoryTest final : public QObject {
    Q_OBJECT

private slots:
    void savesAndRestoresContent();
    void fallsBackFromCorruptValues();
};

void SettingsRepositoryTest::savesAndRestoresContent() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString fileName = directory.filePath(QStringLiteral("settings.ini"));
    {
        SettingsRepository repository(fileName);
        DeviceInfo device;
        device.id = QStringLiteral("lamp-1");
        device.ipAddress = QStringLiteral("192.168.1.20");
        device.name = QStringLiteral("Desk");
        device.capabilities.methods.insert(QStringLiteral("set_power"));
        repository.saveDevices({device});

        EffectPreset effect;
        effect.id = QUuid::createUuid();
        effect.name = QStringLiteral("Reading");
        effect.steps.append({500, EffectStep::Mode::ColorTemperature, 4000, 80});
        repository.saveEffects({effect});

        ScheduledAction schedule;
        schedule.id = QUuid::createUuid();
        schedule.name = QStringLiteral("Morning");
        schedule.deviceId = device.id;
        schedule.time = QTime(7, 30);
        schedule.days.insert(Qt::Monday);
        repository.saveSchedules({schedule});
        repository.setValue(QStringLiteral("ui/theme"), QStringLiteral("dark"));
        repository.sync();
        QCOMPARE(repository.status(), QSettings::NoError);
    }
    {
        SettingsRepository repository(fileName);
        QCOMPARE(repository.schemaVersion(), 1);
        QCOMPARE(repository.loadDevices().first().name, QStringLiteral("Desk"));
        QCOMPARE(repository.loadEffects().first().steps.first().value, 4000);
        QCOMPARE(repository.loadSchedules().first().time, QTime(7, 30));
        QCOMPARE(repository.value(QStringLiteral("ui/theme")).toString(), QStringLiteral("dark"));
    }
}

void SettingsRepositoryTest::fallsBackFromCorruptValues() {
    QTemporaryDir directory;
    SettingsRepository repository(directory.filePath(QStringLiteral("bad.ini")));
    repository.setValue(QStringLiteral("devices/remembered"), QByteArray("{broken"));
    repository.setValue(QStringLiteral("effects/custom"), QByteArray("null"));
    QVERIFY(repository.loadDevices().isEmpty());
    QVERIFY(repository.loadEffects().isEmpty());
}

QTEST_MAIN(SettingsRepositoryTest)
#include "test_settings_repository.moc"

