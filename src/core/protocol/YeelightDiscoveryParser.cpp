#include "YeelightDiscoveryParser.h"

#include <QHash>
#include <QHostAddress>
#include <QRegularExpression>
#include <QUrl>

namespace {
int integerValue(const QHash<QString, QString>& headers, const QString& key) {
    bool ok = false;
    const int value = headers.value(key).toInt(&ok);
    return ok ? value : -1;
}

PowerState powerValue(const QString& value) {
    if (value.compare(QStringLiteral("on"), Qt::CaseInsensitive) == 0) {
        return PowerState::On;
    }
    if (value.compare(QStringLiteral("off"), Qt::CaseInsensitive) == 0) {
        return PowerState::Off;
    }
    return PowerState::Unknown;
}

ColorMode colorModeValue(int value) {
    switch (value) {
    case 1: return ColorMode::Rgb;
    case 2: return ColorMode::ColorTemperature;
    case 3: return ColorMode::Hsv;
    default: return ColorMode::Unknown;
    }
}
}

DiscoveryParseResult YeelightDiscoveryParser::parse(const QByteArray& datagram) {
    QHash<QString, QString> headers;
    const auto lines = datagram.split('\n');
    for (const auto& rawLine : lines) {
        const QByteArray line = rawLine.trimmed();
        const qsizetype separator = line.indexOf(':');
        if (separator <= 0) {
            continue;
        }
        const QString key = QString::fromLatin1(line.left(separator)).trimmed().toLower();
        const QString value = QString::fromUtf8(line.mid(separator + 1)).trimmed();
        headers.insert(key, value);
    }

    const QString location = headers.value(QStringLiteral("location"));
    static const QRegularExpression locationPattern(
        QStringLiteral(R"(^yeelight://(?:\[([0-9a-fA-F:]+)\]|([^:]+)):(\d{1,5})$)")
    );
    const auto match = locationPattern.match(location);
    if (!match.hasMatch()) {
        return {false, {}, {}, QStringLiteral("Discovery response has an invalid location.")};
    }

    bool portOk = false;
    const int port = match.captured(3).toInt(&portOk);
    const QString address = match.captured(1).isEmpty()
        ? match.captured(2)
        : match.captured(1);
    if (!portOk || port < 1 || port > 65535 || QHostAddress(address).isNull()) {
        return {false, {}, {}, QStringLiteral("Discovery response has an invalid address or port.")};
    }

    DeviceCapabilities capabilities;
    const auto methods = headers.value(QStringLiteral("support"))
        .split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const auto& method : methods) {
        capabilities.methods.insert(method.trimmed());
    }

    DeviceInfo device;
    device.id = headers.value(QStringLiteral("id"));
    device.ipAddress = address;
    device.port = static_cast<quint16>(port);
    device.model = headers.value(QStringLiteral("model"));
    device.firmwareVersion = headers.value(QStringLiteral("fw_ver"));
    device.name = QUrl::fromPercentEncoding(
        headers.value(QStringLiteral("name")).toUtf8()
    );
    device.capabilities = capabilities;

    DeviceState state;
    state.power = powerValue(headers.value(QStringLiteral("power")));
    state.brightness = integerValue(headers, QStringLiteral("bright"));
    state.colorTemperature = integerValue(headers, QStringLiteral("ct"));
    state.rgb = integerValue(headers, QStringLiteral("rgb"));
    state.hue = integerValue(headers, QStringLiteral("hue"));
    state.saturation = integerValue(headers, QStringLiteral("sat"));
    state.colorMode = colorModeValue(integerValue(headers, QStringLiteral("color_mode")));
    state.reachable = true;
    state.lastSeen = QDateTime::currentDateTimeUtc();
    return {true, device, state, {}};
}
