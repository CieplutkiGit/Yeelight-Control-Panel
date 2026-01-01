#pragma once

#include "../model/EffectPreset.h"

#include <QColor>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

struct CommandResult {
    bool success = false;
    QJsonObject command;
    QString error;
};

class YeelightCommand final {
public:
    static CommandResult getProperties(int id, const QStringList& properties);
    static CommandResult setPower(int id, bool on, int durationMs);
    static CommandResult toggle(int id);
    static CommandResult setBrightness(int id, int brightness, int durationMs);
    static CommandResult setRgb(int id, const QColor& color, int durationMs);
    static CommandResult setHsv(int id, int hue, int saturation, int durationMs);
    static CommandResult setColorTemperature(int id, int temperature, int durationMs);
    static CommandResult startColorFlow(int id, const EffectPreset& preset);
    static CommandResult stopColorFlow(int id);
    static CommandResult setName(int id, const QString& name);
    static CommandResult startMusicMode(
        int id,
        const QHostAddress& callbackAddress,
        quint16 callbackPort
    );
    static CommandResult stopMusicMode(int id);
    static CommandResult getTimers(int id, int timerType);
    static CommandResult addPowerOffTimer(int id, int minutes);
    static CommandResult deleteTimer(int id, int timerType);

    static QByteArray serialize(const QJsonObject& command);

private:
    static CommandResult make(int id, const QString& method, const QJsonArray& params);
    static CommandResult invalid(const QString& error);
    static QString transition(int durationMs);
};
