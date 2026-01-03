#include "AutomationEngine.h"

#include "../logging/AppLogger.h"

#include <algorithm>

AutomationEngine::AutomationEngine(
    DeviceManager* devices,
    SettingsRepository* settings,
    QObject* parent
)
    : QObject(parent)
    , devices_(devices)
    , settings_(settings)
    , schedules_(settings == nullptr ? QList<ScheduledAction>{}
                                     : settings->loadSchedules())
    , nowProvider_([] { return QDateTime::currentDateTime(); }) {
    timer_.setInterval(15000);
    connect(&timer_, &QTimer::timeout, this, &AutomationEngine::tick);
    timer_.start();
}

QList<ScheduledAction> AutomationEngine::schedules() const {
    return schedules_;
}

void AutomationEngine::setSchedules(const QList<ScheduledAction>& schedules) {
    schedules_ = schedules;
    persist();
    emit schedulesChanged();
}

void AutomationEngine::setNowProvider(std::function<QDateTime()> provider) {
    nowProvider_ = std::move(provider);
}

void AutomationEngine::tick() {
    const QDateTime now = nowProvider_();
    bool changed = false;
    for (auto& action : schedules_) {
        if (!action.enabled
            || !action.days.contains(static_cast<Qt::DayOfWeek>(
                now.date().dayOfWeek()
            ))
            || action.time.hour() != now.time().hour()
            || action.time.minute() != now.time().minute()) {
            continue;
        }
        if (action.lastExecuted.isValid()
            && action.lastExecuted.date() == now.date()
            && action.lastExecuted.time().hour() == now.time().hour()
            && action.lastExecuted.time().minute() == now.time().minute()) {
            continue;
        }
        execute(action, now);
        changed = true;
    }
    if (changed) {
        persist();
        emit schedulesChanged();
    }
}

bool AutomationEngine::runNow(const QUuid& actionId) {
    for (auto& action : schedules_) {
        if (action.id == actionId) {
            const bool success = execute(action, nowProvider_());
            persist();
            emit schedulesChanged();
            return success;
        }
    }
    return false;
}

bool AutomationEngine::execute(ScheduledAction& action, const QDateTime& now) {
    action.lastExecuted = now;
    auto* controller = devices_->device(action.deviceId);
    if (controller == nullptr || !controller->state().reachable) {
        const QString message = QStringLiteral("Automation target is offline.");
        AppLogger::instance().log(
            AppLogger::Severity::Warning,
            QStringLiteral("automation"),
            message,
            action.deviceId
        );
        emit actionFinished(action.id, false, message);
        return false;
    }

    switch (action.type) {
    case ScheduledActionType::PowerOn:
        controller->setPower(true);
        break;
    case ScheduledActionType::PowerOff:
        controller->setPower(false);
        break;
    case ScheduledActionType::Toggle:
        controller->toggle();
        break;
    case ScheduledActionType::SetBrightness: {
        bool ok = false;
        const int brightness = action.value.toInt(&ok);
        if (!ok || brightness < 1 || brightness > 100) {
            const QString message = QStringLiteral("Scheduled brightness is invalid.");
            emit actionFinished(action.id, false, message);
            return false;
        }
        controller->setBrightness(brightness);
        break;
    }
    case ScheduledActionType::ApplyPreset: {
        const QUuid presetId(action.value.toString());
        const QList<EffectPreset> effects = settings_ == nullptr
            ? QList<EffectPreset>{}
            : settings_->loadEffects();
        const auto found = std::find_if(
            effects.cbegin(),
            effects.cend(),
            [&presetId](const EffectPreset& effect) { return effect.id == presetId; }
        );
        if (found == effects.cend()) {
            const QString message = QStringLiteral("Scheduled effect preset was not found.");
            emit actionFinished(action.id, false, message);
            return false;
        }
        controller->startEffect(*found);
        break;
    }
    }

    const QString message = QStringLiteral("Scheduled action sent to the device.");
    AppLogger::instance().log(
        AppLogger::Severity::Info,
        QStringLiteral("automation"),
        message,
        action.deviceId
    );
    emit actionFinished(action.id, true, message);
    return true;
}

void AutomationEngine::persist() {
    if (settings_ != nullptr) {
        settings_->saveSchedules(schedules_);
        settings_->sync();
    }
}
