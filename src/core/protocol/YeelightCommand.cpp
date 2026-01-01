#include "YeelightCommand.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace {
bool validId(int id) {
    return id > 0;
}

bool validDuration(int durationMs) {
    return durationMs >= 30 && durationMs <= 5000;
}
}

CommandResult YeelightCommand::make(
    int id,
    const QString& method,
    const QJsonArray& params
) {
    if (!validId(id)) {
        return invalid(QStringLiteral("Request ID must be positive."));
    }
    return {
        true,
        QJsonObject{
            {QStringLiteral("id"), id},
            {QStringLiteral("method"), method},
            {QStringLiteral("params"), params}
        },
        {}
    };
}

CommandResult YeelightCommand::invalid(const QString& error) {
    return {false, {}, error};
}

QString YeelightCommand::transition(int durationMs) {
    return durationMs == 0 ? QStringLiteral("sudden") : QStringLiteral("smooth");
}

CommandResult YeelightCommand::getProperties(
    int id,
    const QStringList& properties
) {
    if (properties.isEmpty()) {
        return invalid(QStringLiteral("At least one property is required."));
    }
    QJsonArray params;
    for (const auto& property : properties) {
        if (property.trimmed().isEmpty()) {
            return invalid(QStringLiteral("Property names cannot be empty."));
        }
        params.append(property);
    }
    return make(id, QStringLiteral("get_prop"), params);
}

CommandResult YeelightCommand::setPower(int id, bool on, int durationMs) {
    if (!validDuration(durationMs)) {
        return invalid(QStringLiteral("Transition duration must be between 30 and 5000 ms."));
    }
    return make(id, QStringLiteral("set_power"), {
        on ? QStringLiteral("on") : QStringLiteral("off"),
        transition(durationMs),
        durationMs
    });
}

CommandResult YeelightCommand::toggle(int id) {
    return make(id, QStringLiteral("toggle"), {});
}

CommandResult YeelightCommand::setBrightness(
    int id,
    int brightness,
    int durationMs
) {
    if (brightness < 1 || brightness > 100) {
        return invalid(QStringLiteral("Brightness must be between 1 and 100."));
    }
    if (!validDuration(durationMs)) {
        return invalid(QStringLiteral("Transition duration must be between 30 and 5000 ms."));
    }
    return make(id, QStringLiteral("set_bright"), {
        brightness,
        transition(durationMs),
        durationMs
    });
}

CommandResult YeelightCommand::setRgb(int id, const QColor& color, int durationMs) {
    if (!color.isValid()) {
        return invalid(QStringLiteral("RGB color is invalid."));
    }
    if (!validDuration(durationMs)) {
        return invalid(QStringLiteral("Transition duration must be between 30 and 5000 ms."));
    }
    const int rgb = color.red() * 65536 + color.green() * 256 + color.blue();
    return make(id, QStringLiteral("set_rgb"), {
        rgb,
        transition(durationMs),
        durationMs
    });
}

CommandResult YeelightCommand::setHsv(
    int id,
    int hue,
    int saturation,
    int durationMs
) {
    if (hue < 0 || hue > 359 || saturation < 0 || saturation > 100) {
        return invalid(QStringLiteral("HSV values are outside the supported range."));
    }
    if (!validDuration(durationMs)) {
        return invalid(QStringLiteral("Transition duration must be between 30 and 5000 ms."));
    }
    return make(id, QStringLiteral("set_hsv"), {
        hue,
        saturation,
        transition(durationMs),
        durationMs
    });
}

CommandResult YeelightCommand::setColorTemperature(
    int id,
    int temperature,
    int durationMs
) {
    if (temperature < 1700 || temperature > 6500) {
        return invalid(QStringLiteral("Color temperature must be between 1700 and 6500 K."));
    }
    if (!validDuration(durationMs)) {
        return invalid(QStringLiteral("Transition duration must be between 30 and 5000 ms."));
    }
    return make(id, QStringLiteral("set_ct_abx"), {
        temperature,
        transition(durationMs),
        durationMs
    });
}

CommandResult YeelightCommand::startColorFlow(int id, const EffectPreset& preset) {
    const QString error = preset.validationError();
    if (!error.isEmpty()) {
        return invalid(error);
    }
    if (preset.repeatCount < 0 || preset.finishAction < 0 || preset.finishAction > 2) {
        return invalid(QStringLiteral("Effect repeat count or finish action is invalid."));
    }
    QStringList values;
    values.reserve(preset.steps.size() * 4);
    for (const auto& step : preset.steps) {
        const int mode = step.mode == EffectStep::Mode::Rgb
            ? 1
            : (step.mode == EffectStep::Mode::ColorTemperature ? 2 : 7);
        values << QString::number(step.durationMs)
               << QString::number(mode)
               << QString::number(step.value)
               << QString::number(step.brightness);
    }
    return make(id, QStringLiteral("start_cf"), {
        preset.repeatCount,
        preset.finishAction,
        values.join(QLatin1Char(','))
    });
}

CommandResult YeelightCommand::stopColorFlow(int id) {
    return make(id, QStringLiteral("stop_cf"), {});
}

CommandResult YeelightCommand::setName(int id, const QString& name) {
    if (name.toUtf8().size() > 64) {
        return invalid(QStringLiteral("Device name cannot exceed 64 UTF-8 bytes."));
    }
    return make(id, QStringLiteral("set_name"), {name});
}

CommandResult YeelightCommand::startMusicMode(
    int id,
    const QHostAddress& callbackAddress,
    quint16 callbackPort
) {
    if (callbackAddress.isNull() || callbackPort == 0) {
        return invalid(QStringLiteral("Music-mode callback address is invalid."));
    }
    return make(id, QStringLiteral("set_music"), {
        1,
        callbackAddress.toString(),
        callbackPort
    });
}

CommandResult YeelightCommand::stopMusicMode(int id) {
    return make(id, QStringLiteral("set_music"), {0});
}

CommandResult YeelightCommand::getTimers(int id, int timerType) {
    if (timerType < 0) {
        return invalid(QStringLiteral("Timer type cannot be negative."));
    }
    return make(id, QStringLiteral("cron_get"), {timerType});
}

CommandResult YeelightCommand::addPowerOffTimer(int id, int minutes) {
    if (minutes < 1 || minutes > 1440) {
        return invalid(QStringLiteral("Power-off timer must be between 1 and 1440 minutes."));
    }
    return make(id, QStringLiteral("cron_add"), {0, minutes});
}

CommandResult YeelightCommand::deleteTimer(int id, int timerType) {
    if (timerType < 0) {
        return invalid(QStringLiteral("Timer type cannot be negative."));
    }
    return make(id, QStringLiteral("cron_del"), {timerType});
}

QByteArray YeelightCommand::serialize(const QJsonObject& command) {
    return QJsonDocument(command).toJson(QJsonDocument::Compact) + "\r\n";
}

