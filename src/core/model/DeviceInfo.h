#pragma once

#include "DeviceCapabilities.h"

#include <QString>
#include <QtGlobal>

struct DeviceInfo {
    QString id;
    QString ipAddress;
    quint16 port = 55443;
    QString model;
    QString firmwareVersion;
    QString name;
    DeviceCapabilities capabilities;

    [[nodiscard]] QString stableId() const {
        return id.isEmpty()
            ? QStringLiteral("%1:%2").arg(ipAddress).arg(port)
            : id;
    }
};

