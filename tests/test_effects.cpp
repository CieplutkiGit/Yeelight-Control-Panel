#include "../src/core/model/EffectPreset.h"
#include "../src/core/protocol/YeelightCommand.h"
#include "../src/ui/pages/EffectsPage.h"

#include <QtTest>

class EffectsTest final : public QObject {
    Q_OBJECT

private slots:
    void builtInsAreValid();
    void invalidStepsAreRejected();
    void serializesFlowExpression();
};

void EffectsTest::builtInsAreValid() {
    const auto effects = EffectsPage::builtInEffects();
    QCOMPARE(effects.size(), 8);
    for (const auto& effect : effects) {
        QVERIFY2(effect.validationError().isEmpty(), qPrintable(effect.name));
    }
}

void EffectsTest::invalidStepsAreRejected() {
    EffectPreset effect;
    QVERIFY(!effect.validationError().isEmpty());
    effect.steps.append({49, EffectStep::Mode::Rgb, 0, 100});
    QVERIFY(!effect.validationError().isEmpty());
    effect.steps[0].durationMs = 50;
    effect.steps[0].brightness = 0;
    QVERIFY(!effect.validationError().isEmpty());
}

void EffectsTest::serializesFlowExpression() {
    EffectPreset effect;
    effect.steps.append({500, EffectStep::Mode::Rgb, 0x102030, 75});
    const auto command = YeelightCommand::startColorFlow(1, effect);
    QVERIFY(command.success);
    QCOMPARE(
        command.command.value(QStringLiteral("params")).toArray().at(2).toString(),
        QStringLiteral("500,1,1056816,75")
    );
}

QTEST_MAIN(EffectsTest)
#include "test_effects.moc"

