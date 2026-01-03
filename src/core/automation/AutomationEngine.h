#pragma once

#include "../device/DeviceManager.h"
#include "../model/ScheduledAction.h"
#include "../persistence/SettingsRepository.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QTimer>

#include <functional>

class AutomationEngine final : public QObject {
    Q_OBJECT

public:
    explicit AutomationEngine(
        DeviceManager* devices,
        SettingsRepository* settings,
        QObject* parent = nullptr
    );

    [[nodiscard]] QList<ScheduledAction> schedules() const;
    void setSchedules(const QList<ScheduledAction>& schedules);
    void setNowProvider(std::function<QDateTime()> provider);
    void tick();
    bool runNow(const QUuid& actionId);

signals:
    void schedulesChanged();
    void actionFinished(
        const QUuid& actionId,
        bool success,
        const QString& message
    );

private:
    bool execute(ScheduledAction& action, const QDateTime& now);
    void persist();

    DeviceManager* devices_;
    SettingsRepository* settings_;
    QList<ScheduledAction> schedules_;
    QTimer timer_;
    std::function<QDateTime()> nowProvider_;
};

