#pragma once

#include <QDateTime>
#include <QSet>
#include <QTime>
#include <QUuid>
#include <QVariant>

enum class ScheduledActionType {
    PowerOn,
    PowerOff,
    Toggle,
    SetBrightness,
    ApplyPreset
};

struct ScheduledAction {
    QUuid id;
    QString name;
    QString deviceId;
    ScheduledActionType type = ScheduledActionType::PowerOn;
    QVariant value;
    QTime time;
    QSet<Qt::DayOfWeek> days;
    bool enabled = true;
    QDateTime lastExecuted;
};

