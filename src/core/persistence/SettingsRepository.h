#pragma once

#include "../model/DeviceInfo.h"
#include "../model/EffectPreset.h"
#include "../model/ScheduledAction.h"

#include <QByteArray>
#include <QList>
#include <QSettings>
#include <QVariant>

#include <memory>

class SettingsRepository final {
public:
    explicit SettingsRepository(const QString& fileName = {});

    void saveDevices(const QList<DeviceInfo>& devices);
    [[nodiscard]] QList<DeviceInfo> loadDevices() const;

    void saveEffects(const QList<EffectPreset>& effects);
    [[nodiscard]] QList<EffectPreset> loadEffects() const;

    void saveSchedules(const QList<ScheduledAction>& schedules);
    [[nodiscard]] QList<ScheduledAction> loadSchedules() const;

    void setValue(const QString& key, const QVariant& value);
    [[nodiscard]] QVariant value(
        const QString& key,
        const QVariant& fallback = {}
    ) const;
    [[nodiscard]] int schemaVersion() const;
    [[nodiscard]] QSettings::Status status() const;
    void sync();

private:
    void initializeSchema();
    std::unique_ptr<QSettings> settings_;
};

