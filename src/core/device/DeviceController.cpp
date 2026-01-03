#include "DeviceController.h"

#include <QJsonObject>

namespace {
int propertyInteger(const QVariantMap& properties, const QString& key, int fallback) {
    if (!properties.contains(key)) {
        return fallback;
    }
    bool ok = false;
    const int value = properties.value(key).toString().toInt(&ok);
    return ok ? value : fallback;
}
}

DeviceController::DeviceController(
    const DeviceInfo& info,
    const DeviceState& state,
    QObject* parent
)
    : QObject(parent)
    , info_(info)
    , state_(state) {
    createConnection();
}

DeviceInfo DeviceController::info() const {
    return info_;
}

DeviceState DeviceController::state() const {
    return state_;
}

YeelightConnection::Status DeviceController::connectionStatus() const {
    return connection_->status();
}

void DeviceController::updateDiscovery(const DeviceInfo& info, const DeviceState& state) {
    const bool endpointChanged = info_.ipAddress != info.ipAddress || info_.port != info.port;
    const QString preservedName = info_.name;
    info_ = info;
    if (!preservedName.isEmpty()) {
        info_.name = preservedName;
    }
    state_ = state;
    emit infoChanged(info_);
    emit stateChanged(state_);
    if (endpointChanged) {
        connection_->disconnectFromDevice();
        connection_->deleteLater();
        createConnection();
    }
}

void DeviceController::markOffline() {
    if (!state_.reachable) {
        return;
    }
    state_.reachable = false;
    emit stateChanged(state_);
}

void DeviceController::connectDevice() {
    connection_->connectToDevice();
}

void DeviceController::disconnectDevice() {
    connection_->disconnectFromDevice();
}

void DeviceController::refreshState() {
    if (!requireCapability(QStringLiteral("get_prop"))) {
        return;
    }
    sendResult(YeelightCommand::getProperties(nextRequestId_++, {
        QStringLiteral("power"), QStringLiteral("bright"), QStringLiteral("ct"),
        QStringLiteral("rgb"), QStringLiteral("hue"), QStringLiteral("sat"),
        QStringLiteral("color_mode"), QStringLiteral("flowing"),
        QStringLiteral("delayoff"), QStringLiteral("music_on"),
        QStringLiteral("name")
    }));
}

void DeviceController::setPower(bool on) {
    if (!requireCapability(QStringLiteral("set_power"))) {
        return;
    }
    const DeviceState previous = state_;
    state_.power = on ? PowerState::On : PowerState::Off;
    const auto result = YeelightCommand::setPower(
        nextRequestId_++,
        on,
        transitionDurationMs_
    );
    optimisticStates_.insert(result.command.value(QStringLiteral("id")).toInt(), previous);
    emit stateChanged(state_);
    sendResult(result);
}

void DeviceController::toggle() {
    if (!requireCapability(QStringLiteral("toggle"))) {
        return;
    }
    sendResult(YeelightCommand::toggle(nextRequestId_++));
}

void DeviceController::setBrightness(int value) {
    if (!requireCapability(QStringLiteral("set_bright"))) {
        return;
    }
    const DeviceState previous = state_;
    state_.brightness = value;
    const auto result = YeelightCommand::setBrightness(
        nextRequestId_++,
        value,
        transitionDurationMs_
    );
    if (result.success) {
        optimisticStates_.insert(result.command.value(QStringLiteral("id")).toInt(), previous);
        emit stateChanged(state_);
    }
    sendResult(result);
}

void DeviceController::setRgb(const QColor& color) {
    if (!requireCapability(QStringLiteral("set_rgb"))) {
        return;
    }
    const DeviceState previous = state_;
    state_.rgb = color.red() * 65536 + color.green() * 256 + color.blue();
    state_.colorMode = ColorMode::Rgb;
    const auto result = YeelightCommand::setRgb(
        nextRequestId_++,
        color,
        transitionDurationMs_
    );
    if (result.success) {
        optimisticStates_.insert(result.command.value(QStringLiteral("id")).toInt(), previous);
        emit stateChanged(state_);
    }
    sendResult(result);
}

void DeviceController::setHsv(int hue, int saturation) {
    if (!requireCapability(QStringLiteral("set_hsv"))) {
        return;
    }
    const DeviceState previous = state_;
    state_.hue = hue;
    state_.saturation = saturation;
    state_.colorMode = ColorMode::Hsv;
    const auto result = YeelightCommand::setHsv(
        nextRequestId_++,
        hue,
        saturation,
        transitionDurationMs_
    );
    if (result.success) {
        optimisticStates_.insert(result.command.value(QStringLiteral("id")).toInt(), previous);
        emit stateChanged(state_);
    }
    sendResult(result);
}

void DeviceController::setColorTemperature(int kelvin) {
    if (!requireCapability(QStringLiteral("set_ct_abx"))) {
        return;
    }
    const DeviceState previous = state_;
    state_.colorTemperature = kelvin;
    state_.colorMode = ColorMode::ColorTemperature;
    const auto result = YeelightCommand::setColorTemperature(
        nextRequestId_++,
        kelvin,
        transitionDurationMs_
    );
    if (result.success) {
        optimisticStates_.insert(result.command.value(QStringLiteral("id")).toInt(), previous);
        emit stateChanged(state_);
    }
    sendResult(result);
}

void DeviceController::startEffect(const EffectPreset& preset) {
    if (!requireCapability(QStringLiteral("start_cf"))) {
        return;
    }
    sendResult(YeelightCommand::startColorFlow(nextRequestId_++, preset));
}

void DeviceController::stopEffect() {
    if (!requireCapability(QStringLiteral("stop_cf"))) {
        return;
    }
    sendResult(YeelightCommand::stopColorFlow(nextRequestId_++));
}

void DeviceController::setLocalName(const QString& name) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        emit commandError(QStringLiteral("Local device name cannot be empty."));
        return;
    }
    info_.name = trimmed;
    emit infoChanged(info_);
}

void DeviceController::setDeviceName(const QString& name) {
    if (!requireCapability(QStringLiteral("set_name"))) {
        return;
    }
    sendResult(YeelightCommand::setName(nextRequestId_++, name));
}

void DeviceController::setTransitionDuration(int durationMs) {
    if (durationMs < 30 || durationMs > 5000) {
        emit commandError(QStringLiteral(
            "Transition duration must be between 30 and 5000 ms."
        ));
        return;
    }
    transitionDurationMs_ = durationMs;
}

int DeviceController::sendRaw(const QString& method, const QJsonArray& parameters) {
    if (method.trimmed().isEmpty()) {
        emit commandError(QStringLiteral("Raw command method cannot be empty."));
        return -1;
    }
    const int requestId = nextRequestId_++;
    QJsonObject command{
        {QStringLiteral("id"), requestId},
        {QStringLiteral("method"), method},
        {QStringLiteral("params"), parameters}
    };
    emit rawRequest(requestId, YeelightCommand::serialize(command));
    connection_->send(command);
    return requestId;
}

bool DeviceController::requireCapability(const QString& method) {
    if (info_.capabilities.supports(method)) {
        return true;
    }
    emit unsupportedOperation(
        QStringLiteral("This device does not advertise support for %1.").arg(method)
    );
    return false;
}

void DeviceController::sendResult(const CommandResult& result) {
    if (!result.success) {
        emit commandError(result.error);
        return;
    }
    connection_->send(result.command);
}

void DeviceController::createConnection() {
    connection_ = new YeelightConnection(info_, this);
    connect(connection_, &YeelightConnection::statusChanged,
        this, &DeviceController::connectionStatusChanged);
    connect(connection_, &YeelightConnection::propertiesChanged,
        this, &DeviceController::applyProperties);
    connect(connection_, &YeelightConnection::responseReceived, this,
        [this](int id, const QJsonArray& result) {
            optimisticStates_.remove(id);
            emit rawResponse(id, result);
        });
    connect(connection_, &YeelightConnection::commandFailed, this,
        [this](int id, int, const QString& message) {
            if (optimisticStates_.contains(id)) {
                state_ = optimisticStates_.take(id);
                emit stateChanged(state_);
            }
            emit commandError(message);
        });
    connect(connection_, &YeelightConnection::protocolError,
        this, &DeviceController::commandError);
}

void DeviceController::applyProperties(const QVariantMap& properties) {
    const QString power = properties.value(QStringLiteral("power")).toString();
    if (power == QStringLiteral("on")) {
        state_.power = PowerState::On;
    } else if (power == QStringLiteral("off")) {
        state_.power = PowerState::Off;
    }
    state_.brightness = propertyInteger(properties, QStringLiteral("bright"), state_.brightness);
    state_.colorTemperature = propertyInteger(properties, QStringLiteral("ct"), state_.colorTemperature);
    state_.rgb = propertyInteger(properties, QStringLiteral("rgb"), state_.rgb);
    state_.hue = propertyInteger(properties, QStringLiteral("hue"), state_.hue);
    state_.saturation = propertyInteger(properties, QStringLiteral("sat"), state_.saturation);
    state_.flowing = propertyInteger(properties, QStringLiteral("flowing"), state_.flowing ? 1 : 0) == 1;
    state_.musicMode = propertyInteger(properties, QStringLiteral("music_on"), state_.musicMode ? 1 : 0) == 1;
    state_.delayedOffMinutes = propertyInteger(
        properties,
        QStringLiteral("delayoff"),
        state_.delayedOffMinutes
    );
    state_.reachable = true;
    state_.lastSeen = QDateTime::currentDateTimeUtc();
    emit stateChanged(state_);
}
