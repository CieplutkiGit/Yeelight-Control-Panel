#pragma once

#include "../model/DeviceInfo.h"
#include "../model/DeviceState.h"
#include "../model/EffectPreset.h"
#include "../network/YeelightConnection.h"
#include "../protocol/YeelightCommand.h"

#include <QColor>
#include <QHash>
#include <QObject>

class DeviceController final : public QObject {
    Q_OBJECT

public:
    explicit DeviceController(
        const DeviceInfo& info,
        const DeviceState& state = {},
        QObject* parent = nullptr
    );

    [[nodiscard]] DeviceInfo info() const;
    [[nodiscard]] DeviceState state() const;
    [[nodiscard]] YeelightConnection::Status connectionStatus() const;

    void updateDiscovery(const DeviceInfo& info, const DeviceState& state);
    void markOffline();
    void connectDevice();
    void disconnectDevice();
    void refreshState();
    void setPower(bool on);
    void toggle();
    void setBrightness(int value);
    void setRgb(const QColor& color);
    void setHsv(int hue, int saturation);
    void setColorTemperature(int kelvin);
    void startEffect(const EffectPreset& preset);
    void stopEffect();
    void setDeviceName(const QString& name);
    void sendRaw(const QString& method, const QJsonArray& parameters);

signals:
    void infoChanged(const DeviceInfo& info);
    void stateChanged(const DeviceState& state);
    void connectionStatusChanged(YeelightConnection::Status status);
    void unsupportedOperation(const QString& message);
    void commandError(const QString& message);
    void rawResponse(int requestId, const QJsonArray& result);

private:
    bool requireCapability(const QString& method);
    void sendResult(const CommandResult& result, bool optimistic = false);
    void createConnection();
    void applyProperties(const QVariantMap& properties);

    DeviceInfo info_;
    DeviceState state_;
    YeelightConnection* connection_ = nullptr;
    QHash<int, DeviceState> optimisticStates_;
    int nextRequestId_ = 1;
    int transitionDurationMs_ = 300;
};

