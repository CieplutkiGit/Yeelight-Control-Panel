#pragma once

#include <QSet>
#include <QString>

struct DeviceCapabilities {
    QSet<QString> methods;

    [[nodiscard]] bool supports(const QString& method) const {
        return methods.contains(method);
    }

    [[nodiscard]] bool supportsPower() const {
        return supports(QStringLiteral("set_power")) || supports(QStringLiteral("toggle"));
    }
    [[nodiscard]] bool supportsBrightness() const {
        return supports(QStringLiteral("set_bright"));
    }
    [[nodiscard]] bool supportsRgb() const {
        return supports(QStringLiteral("set_rgb"));
    }
    [[nodiscard]] bool supportsHsv() const {
        return supports(QStringLiteral("set_hsv"));
    }
    [[nodiscard]] bool supportsColorTemperature() const {
        return supports(QStringLiteral("set_ct_abx"));
    }
    [[nodiscard]] bool supportsColorFlow() const {
        return supports(QStringLiteral("start_cf")) && supports(QStringLiteral("stop_cf"));
    }
    [[nodiscard]] bool supportsTimers() const {
        return supports(QStringLiteral("cron_add"))
            && supports(QStringLiteral("cron_get"))
            && supports(QStringLiteral("cron_del"));
    }
    [[nodiscard]] bool supportsMusicMode() const {
        return supports(QStringLiteral("set_music"));
    }
    [[nodiscard]] bool supportsBackgroundLight() const {
        return supports(QStringLiteral("bg_set_power"));
    }
    [[nodiscard]] bool supportsSegmentControl() const {
        static const QSet<QString> knownMethods{
            QStringLiteral("set_segment_rgb"),
            QStringLiteral("set_segment_hsv"),
            QStringLiteral("set_segment_ct")
        };
        for (const auto& method : knownMethods) {
            if (methods.contains(method)) {
                return true;
            }
        }
        return false;
    }
};

