#pragma once

#include <QDateTime>

enum class PowerState {
    Unknown,
    Off,
    On
};

enum class ColorMode {
    Unknown,
    ColorTemperature,
    Rgb,
    Hsv,
    ColorFlow
};

struct DeviceState {
    PowerState power = PowerState::Unknown;
    int brightness = -1;
    int colorTemperature = -1;
    int rgb = -1;
    int hue = -1;
    int saturation = -1;
    ColorMode colorMode = ColorMode::Unknown;
    bool flowing = false;
    bool musicMode = false;
    int delayedOffMinutes = 0;
    bool reachable = false;
    QDateTime lastSeen;
};

