#include "../src/core/protocol/YeelightCommand.h"

#include <QJsonArray>
#include <QtTest>

class CommandSerializationTest final : public QObject {
    Q_OBJECT

private slots:
    void serializesExactlyOneTerminator();
    void createsStandardCommands();
    void convertsRgbToInteger();
    void rejectsInvalidRanges();
    void createsEffectsAndUtilityCommands();
};

void CommandSerializationTest::serializesExactlyOneTerminator() {
    const auto result = YeelightCommand::toggle(42);
    QVERIFY(result.success);
    const QByteArray wire = YeelightCommand::serialize(result.command);
    QCOMPARE(wire.count("\r\n"), 1);
    QVERIFY(wire.endsWith("\r\n"));
    QVERIFY(!wire.left(wire.size() - 2).contains('\n'));
}

void CommandSerializationTest::createsStandardCommands() {
    const auto power = YeelightCommand::setPower(1, true, 300);
    QCOMPARE(power.command.value("method").toString(), QStringLiteral("set_power"));
    QCOMPARE(power.command.value("id").toInt(), 1);
    QCOMPARE(power.command.value("params").toArray().at(0).toString(), QStringLiteral("on"));

    QCOMPARE(
        YeelightCommand::getProperties(2, {"power"}).command.value("method").toString(),
        QStringLiteral("get_prop")
    );
    QCOMPARE(
        YeelightCommand::setBrightness(3, 50, 300).command.value("method").toString(),
        QStringLiteral("set_bright")
    );
    QCOMPARE(
        YeelightCommand::setHsv(4, 120, 80, 300).command.value("method").toString(),
        QStringLiteral("set_hsv")
    );
    QCOMPARE(
        YeelightCommand::setColorTemperature(5, 4000, 300)
            .command.value("method").toString(),
        QStringLiteral("set_ct_abx")
    );
}

void CommandSerializationTest::convertsRgbToInteger() {
    const auto result = YeelightCommand::setRgb(7, QColor(1, 2, 3), 300);
    QVERIFY(result.success);
    QCOMPARE(result.command.value("params").toArray().at(0).toInt(), 1 * 65536 + 2 * 256 + 3);
}

void CommandSerializationTest::rejectsInvalidRanges() {
    QVERIFY(!YeelightCommand::setBrightness(1, 0, 300).success);
    QVERIFY(!YeelightCommand::setHsv(1, 360, 50, 300).success);
    QVERIFY(!YeelightCommand::setColorTemperature(1, 1000, 300).success);
    QVERIFY(!YeelightCommand::setRgb(1, QColor(), 300).success);
    QVERIFY(!YeelightCommand::toggle(0).success);
}

void CommandSerializationTest::createsEffectsAndUtilityCommands() {
    EffectPreset preset;
    preset.steps.append({500, EffectStep::Mode::Rgb, 0xff0000, 100});
    QVERIFY(YeelightCommand::startColorFlow(1, preset).success);
    QVERIFY(YeelightCommand::stopColorFlow(2).success);
    QVERIFY(YeelightCommand::setName(3, QStringLiteral("Desk")).success);
    QVERIFY(YeelightCommand::startMusicMode(4, QHostAddress::LocalHost, 54321).success);
    QVERIFY(YeelightCommand::stopMusicMode(5).success);
    QVERIFY(YeelightCommand::getTimers(6, 0).success);
    QVERIFY(YeelightCommand::addPowerOffTimer(7, 30).success);
    QVERIFY(YeelightCommand::deleteTimer(8, 0).success);
}

QTEST_MAIN(CommandSerializationTest)
#include "test_command_serialization.moc"

