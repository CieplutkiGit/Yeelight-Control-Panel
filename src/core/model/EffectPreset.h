#pragma once

#include <QList>
#include <QString>
#include <QUuid>

struct EffectStep {
    int durationMs = 500;
    enum class Mode {
        Rgb,
        ColorTemperature,
        Sleep
    } mode = Mode::Rgb;
    int value = 0;
    int brightness = 100;
};

struct EffectPreset {
    QUuid id;
    QString name;
    QList<EffectStep> steps;
    int repeatCount = 0;
    int finishAction = 0;

    [[nodiscard]] QString validationError() const {
        if (steps.isEmpty()) {
            return QStringLiteral("An effect requires at least one step.");
        }
        for (const auto& step : steps) {
            if (step.durationMs < 50) {
                return QStringLiteral("Step duration must be at least 50 ms.");
            }
            if (step.brightness < 1 || step.brightness > 100) {
                return QStringLiteral("Step brightness must be between 1 and 100.");
            }
            if (step.mode == EffectStep::Mode::Rgb
                && (step.value < 0 || step.value > 16777215)) {
                return QStringLiteral("RGB values must be between 0 and 16777215.");
            }
            if (step.mode == EffectStep::Mode::ColorTemperature
                && (step.value < 1700 || step.value > 6500)) {
                return QStringLiteral("Color temperature must be between 1700 and 6500 K.");
            }
        }
        return {};
    }
};

