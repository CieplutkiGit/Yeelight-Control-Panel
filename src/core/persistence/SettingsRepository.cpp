#include "SettingsRepository.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr int CurrentSchemaVersion = 1;

QJsonObject deviceToJson(const DeviceInfo& device) {
    QJsonArray methods;
    for (const auto& method : device.capabilities.methods) {
        methods.append(method);
    }
    return {
        {QStringLiteral("id"), device.id},
        {QStringLiteral("ipAddress"), device.ipAddress},
        {QStringLiteral("port"), device.port},
        {QStringLiteral("model"), device.model},
        {QStringLiteral("firmwareVersion"), device.firmwareVersion},
        {QStringLiteral("name"), device.name},
        {QStringLiteral("methods"), methods}
    };
}

DeviceInfo deviceFromJson(const QJsonObject& object) {
    DeviceInfo device;
    device.id = object.value(QStringLiteral("id")).toString();
    device.ipAddress = object.value(QStringLiteral("ipAddress")).toString();
    const int port = object.value(QStringLiteral("port")).toInt(55443);
    device.port = port >= 1 && port <= 65535
        ? static_cast<quint16>(port)
        : quint16(55443);
    device.model = object.value(QStringLiteral("model")).toString();
    device.firmwareVersion = object.value(QStringLiteral("firmwareVersion")).toString();
    device.name = object.value(QStringLiteral("name")).toString();
    for (const auto& method : object.value(QStringLiteral("methods")).toArray()) {
        device.capabilities.methods.insert(method.toString());
    }
    return device;
}

QJsonObject effectToJson(const EffectPreset& effect) {
    QJsonArray steps;
    for (const auto& step : effect.steps) {
        steps.append(QJsonObject{
            {QStringLiteral("durationMs"), step.durationMs},
            {QStringLiteral("mode"), static_cast<int>(step.mode)},
            {QStringLiteral("value"), step.value},
            {QStringLiteral("brightness"), step.brightness}
        });
    }
    return {
        {QStringLiteral("id"), effect.id.toString(QUuid::WithoutBraces)},
        {QStringLiteral("name"), effect.name},
        {QStringLiteral("repeatCount"), effect.repeatCount},
        {QStringLiteral("finishAction"), effect.finishAction},
        {QStringLiteral("steps"), steps}
    };
}

EffectPreset effectFromJson(const QJsonObject& object) {
    EffectPreset effect;
    effect.id = QUuid(object.value(QStringLiteral("id")).toString());
    effect.name = object.value(QStringLiteral("name")).toString();
    effect.repeatCount = object.value(QStringLiteral("repeatCount")).toInt();
    effect.finishAction = object.value(QStringLiteral("finishAction")).toInt();
    for (const auto& value : object.value(QStringLiteral("steps")).toArray()) {
        const auto stepObject = value.toObject();
        EffectStep step;
        step.durationMs = stepObject.value(QStringLiteral("durationMs")).toInt(500);
        const int mode = stepObject.value(QStringLiteral("mode")).toInt();
        if (mode >= static_cast<int>(EffectStep::Mode::Rgb)
            && mode <= static_cast<int>(EffectStep::Mode::Sleep)) {
            step.mode = static_cast<EffectStep::Mode>(mode);
        }
        step.value = stepObject.value(QStringLiteral("value")).toInt();
        step.brightness = stepObject.value(QStringLiteral("brightness")).toInt(100);
        effect.steps.append(step);
    }
    return effect;
}

QJsonObject scheduleToJson(const ScheduledAction& schedule) {
    QJsonArray days;
    for (Qt::DayOfWeek day : schedule.days) {
        days.append(static_cast<int>(day));
    }
    return {
        {QStringLiteral("id"), schedule.id.toString(QUuid::WithoutBraces)},
        {QStringLiteral("name"), schedule.name},
        {QStringLiteral("deviceId"), schedule.deviceId},
        {QStringLiteral("type"), static_cast<int>(schedule.type)},
        {QStringLiteral("value"), QJsonValue::fromVariant(schedule.value)},
        {QStringLiteral("time"), schedule.time.toString(Qt::ISODate)},
        {QStringLiteral("days"), days},
        {QStringLiteral("enabled"), schedule.enabled},
        {QStringLiteral("lastExecuted"), schedule.lastExecuted.toString(Qt::ISODate)}
    };
}

ScheduledAction scheduleFromJson(const QJsonObject& object) {
    ScheduledAction schedule;
    schedule.id = QUuid(object.value(QStringLiteral("id")).toString());
    schedule.name = object.value(QStringLiteral("name")).toString();
    schedule.deviceId = object.value(QStringLiteral("deviceId")).toString();
    const int type = object.value(QStringLiteral("type")).toInt();
    if (type >= static_cast<int>(ScheduledActionType::PowerOn)
        && type <= static_cast<int>(ScheduledActionType::ApplyPreset)) {
        schedule.type = static_cast<ScheduledActionType>(type);
    }
    schedule.value = object.value(QStringLiteral("value")).toVariant();
    schedule.time = QTime::fromString(
        object.value(QStringLiteral("time")).toString(),
        Qt::ISODate
    );
    for (const auto& value : object.value(QStringLiteral("days")).toArray()) {
        const int day = value.toInt();
        if (day >= Qt::Monday && day <= Qt::Sunday) {
            schedule.days.insert(static_cast<Qt::DayOfWeek>(day));
        }
    }
    schedule.enabled = object.value(QStringLiteral("enabled")).toBool(true);
    schedule.lastExecuted = QDateTime::fromString(
        object.value(QStringLiteral("lastExecuted")).toString(),
        Qt::ISODate
    );
    return schedule;
}

QJsonArray parseArray(const QVariant& value) {
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(value.toByteArray(), &error);
    return error.error == QJsonParseError::NoError && document.isArray()
        ? document.array()
        : QJsonArray{};
}
}

SettingsRepository::SettingsRepository(const QString& fileName) {
    if (fileName.isEmpty()) {
        settings_ = std::make_unique<QSettings>(
            QStringLiteral("CieplutkiGit"),
            QStringLiteral("YeelightControlPanel")
        );
    } else {
        settings_ = std::make_unique<QSettings>(fileName, QSettings::IniFormat);
    }
    initializeSchema();
}

void SettingsRepository::saveDevices(const QList<DeviceInfo>& devices) {
    QJsonArray array;
    for (const auto& device : devices) {
        array.append(deviceToJson(device));
    }
    settings_->setValue(
        QStringLiteral("devices/remembered"),
        QJsonDocument(array).toJson(QJsonDocument::Compact)
    );
}

QList<DeviceInfo> SettingsRepository::loadDevices() const {
    QList<DeviceInfo> devices;
    for (const auto& value : parseArray(settings_->value(QStringLiteral("devices/remembered")))) {
        const DeviceInfo device = deviceFromJson(value.toObject());
        if (!device.ipAddress.isEmpty() && device.port != 0) {
            devices.append(device);
        }
    }
    return devices;
}

void SettingsRepository::saveEffects(const QList<EffectPreset>& effects) {
    QJsonArray array;
    for (const auto& effect : effects) {
        array.append(effectToJson(effect));
    }
    settings_->setValue(
        QStringLiteral("effects/custom"),
        QJsonDocument(array).toJson(QJsonDocument::Compact)
    );
}

QList<EffectPreset> SettingsRepository::loadEffects() const {
    QList<EffectPreset> effects;
    for (const auto& value : parseArray(settings_->value(QStringLiteral("effects/custom")))) {
        const EffectPreset effect = effectFromJson(value.toObject());
        if (effect.validationError().isEmpty()) {
            effects.append(effect);
        }
    }
    return effects;
}

void SettingsRepository::saveSchedules(const QList<ScheduledAction>& schedules) {
    QJsonArray array;
    for (const auto& schedule : schedules) {
        array.append(scheduleToJson(schedule));
    }
    settings_->setValue(
        QStringLiteral("automations/schedules"),
        QJsonDocument(array).toJson(QJsonDocument::Compact)
    );
}

QList<ScheduledAction> SettingsRepository::loadSchedules() const {
    QList<ScheduledAction> schedules;
    for (const auto& value : parseArray(settings_->value(
             QStringLiteral("automations/schedules")))) {
        const ScheduledAction schedule = scheduleFromJson(value.toObject());
        if (!schedule.id.isNull() && schedule.time.isValid()) {
            schedules.append(schedule);
        }
    }
    return schedules;
}

void SettingsRepository::setValue(const QString& key, const QVariant& value) {
    settings_->setValue(key, value);
}

QVariant SettingsRepository::value(
    const QString& key,
    const QVariant& fallback
) const {
    return settings_->value(key, fallback);
}

int SettingsRepository::schemaVersion() const {
    return settings_->value(QStringLiteral("settings/schemaVersion")).toInt();
}

QSettings::Status SettingsRepository::status() const {
    return settings_->status();
}

void SettingsRepository::sync() {
    settings_->sync();
}

void SettingsRepository::initializeSchema() {
    const int storedVersion = settings_->value(
        QStringLiteral("settings/schemaVersion"),
        0
    ).toInt();
    if (storedVersion < CurrentSchemaVersion) {
        settings_->setValue(
            QStringLiteral("settings/schemaVersion"),
            CurrentSchemaVersion
        );
    }
}

