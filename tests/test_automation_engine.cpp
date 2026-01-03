#include "../src/core/automation/AutomationEngine.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

class AutomationEngineTest final : public QObject {
    Q_OBJECT

private:
    static ScheduledAction actionAt(const QDateTime& now) {
        ScheduledAction action;
        action.id = QUuid::createUuid();
        action.name = QStringLiteral("Test");
        action.deviceId = QStringLiteral("missing");
        action.type = ScheduledActionType::PowerOff;
        action.time = now.time();
        action.days.insert(static_cast<Qt::DayOfWeek>(now.date().dayOfWeek()));
        return action;
    }

private slots:
    void matchingTimeExecutesOnce();
    void disabledAndWrongDayDoNotExecute();
    void missedActionsAreNotReplayed();
};

void AutomationEngineTest::matchingTimeExecutesOnce() {
    QTemporaryDir directory;
    SettingsRepository settings(directory.filePath(QStringLiteral("settings.ini")));
    DeviceManager devices;
    AutomationEngine engine(&devices, &settings);
    const QDateTime now(QDate(2026, 1, 3), QTime(8, 30));
    engine.setNowProvider([now] { return now; });
    engine.setSchedules({actionAt(now)});
    QSignalSpy finished(&engine, &AutomationEngine::actionFinished);
    engine.tick();
    engine.tick();
    QCOMPARE(finished.size(), 1);
    QCOMPARE(finished.at(0).at(1).toBool(), false);
    QVERIFY(engine.schedules().first().lastExecuted.isValid());
}

void AutomationEngineTest::disabledAndWrongDayDoNotExecute() {
    DeviceManager devices;
    AutomationEngine engine(&devices, nullptr);
    const QDateTime now(QDate(2026, 1, 3), QTime(8, 30));
    ScheduledAction disabled = actionAt(now);
    disabled.enabled = false;
    ScheduledAction wrongDay = actionAt(now);
    wrongDay.days = {Qt::Monday};
    engine.setSchedules({disabled, wrongDay});
    QSignalSpy finished(&engine, &AutomationEngine::actionFinished);
    engine.setNowProvider([now] { return now; });
    engine.tick();
    QCOMPARE(finished.size(), 0);
}

void AutomationEngineTest::missedActionsAreNotReplayed() {
    DeviceManager devices;
    AutomationEngine engine(&devices, nullptr);
    const QDateTime scheduled(QDate(2026, 1, 3), QTime(8, 30));
    const QDateTime later = scheduled.addSecs(60);
    engine.setSchedules({actionAt(scheduled)});
    engine.setNowProvider([later] { return later; });
    QSignalSpy finished(&engine, &AutomationEngine::actionFinished);
    engine.tick();
    QCOMPARE(finished.size(), 0);
}

QTEST_MAIN(AutomationEngineTest)
#include "test_automation_engine.moc"

