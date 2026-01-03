#pragma once

#include "DeviceController.h"
#include "../network/DiscoveryService.h"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QTimer>

class DeviceManager final : public QObject {
    Q_OBJECT

public:
    explicit DeviceManager(QObject* parent = nullptr);

    void startDiscovery();
    void stopDiscovery();
    bool addManualDevice(
        const QString& ipAddress,
        quint16 port = 55443,
        const QString& displayName = {},
        bool remember = true
    );
    void restoreRememberedDevice(const DeviceInfo& info);
    [[nodiscard]] QList<DeviceInfo> rememberedDevices() const;
    void removeRememberedDevice(const QString& stableId);
    void connectAll();
    void disconnectAll();
    QList<DeviceController*> devices() const;
    DeviceController* device(const QString& stableId) const;

signals:
    void deviceAdded(DeviceController* device);
    void deviceRemoved(const QString& stableId);
    void deviceUpdated(DeviceController* device);
    void discoveryStateChanged(bool active);
    void errorOccurred(const QString& message);

private:
    void addOrUpdate(const DeviceInfo& info, const DeviceState& state);

    DiscoveryService discovery_;
    QHash<QString, DeviceController*> devices_;
    QSet<QString> rememberedIds_;
    QTimer offlineTimer_;
};
