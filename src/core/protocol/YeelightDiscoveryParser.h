#pragma once

#include "../model/DeviceInfo.h"
#include "../model/DeviceState.h"

#include <QByteArray>
#include <QString>

struct DiscoveryParseResult {
    bool success = false;
    DeviceInfo device;
    DeviceState state;
    QString error;
};

class YeelightDiscoveryParser final {
public:
    static DiscoveryParseResult parse(const QByteArray& datagram);
};

