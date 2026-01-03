#include "DeviceManager.h"

#include <QDateTime>
#include <QHostAddress>

DeviceManager::DeviceManager(QObject* parent)
    : QObject(parent)
    , discovery_(this) {
    connect(&discovery_, &DiscoveryService::deviceDiscovered,
        this, &DeviceManager::addOrUpdate);
    connect(&discovery_, &DiscoveryService::discoveryError,
        this, &DeviceManager::errorOccurred);
    connect(&discovery_, &DiscoveryService::discoveryFinished, this,
        [this] { emit discoveryStateChanged(false); });
    offlineTimer_.setInterval(5000);
    connect(&offlineTimer_, &QTimer::timeout, this, [this] {
        const QDateTime cutoff = QDateTime::currentDateTimeUtc().addSecs(-30);
        for (auto* controller : devices_) {
            if (controller->state().lastSeen.isValid()
                && controller->state().lastSeen < cutoff) {
                controller->markOffline();
            }
        }
    });
    offlineTimer_.start();
}

void DeviceManager::startDiscovery() {
    emit discoveryStateChanged(true);
    discovery_.start();
    if (!discovery_.isDiscovering()) {
        emit discoveryStateChanged(false);
    }
}

void DeviceManager::stopDiscovery() {
    discovery_.stop();
}

bool DeviceManager::addManualDevice(
    const QString& ipAddress,
    quint16 port,
    const QString& displayName,
    bool remember
) {
    const QHostAddress address(ipAddress);
    if (address.isNull() || port == 0) {
        emit errorOccurred(QStringLiteral("Enter a valid IPv4 or IPv6 address and port."));
        return false;
    }
    DeviceInfo info;
    info.ipAddress = address.toString();
    info.port = port;
    info.name = displayName;
    if (remember) {
        rememberedIds_.insert(info.stableId());
    }
    DeviceState state;
    addOrUpdate(info, state);
    return true;
}

void DeviceManager::restoreRememberedDevice(const DeviceInfo& info) {
    if (QHostAddress(info.ipAddress).isNull() || info.port == 0) {
        emit errorOccurred(QStringLiteral("A remembered device has an invalid address."));
        return;
    }
    rememberedIds_.insert(info.stableId());
    addOrUpdate(info, {});
}

QList<DeviceInfo> DeviceManager::rememberedDevices() const {
    QList<DeviceInfo> result;
    for (const auto& id : rememberedIds_) {
        if (auto* controller = devices_.value(id, nullptr)) {
            result.append(controller->info());
        }
    }
    return result;
}

void DeviceManager::removeRememberedDevice(const QString& stableId) {
    rememberedIds_.remove(stableId);
    auto* controller = devices_.take(stableId);
    if (controller == nullptr) {
        return;
    }
    controller->disconnectDevice();
    controller->deleteLater();
    emit deviceRemoved(stableId);
}

void DeviceManager::connectAll() {
    for (auto* controller : devices_) {
        controller->connectDevice();
    }
}

void DeviceManager::disconnectAll() {
    for (auto* controller : devices_) {
        controller->disconnectDevice();
    }
}

QList<DeviceController*> DeviceManager::devices() const {
    return devices_.values();
}

DeviceController* DeviceManager::device(const QString& stableId) const {
    return devices_.value(stableId, nullptr);
}

void DeviceManager::addOrUpdate(const DeviceInfo& info, const DeviceState& state) {
    QString existingKey;
    if (!info.id.isEmpty() && devices_.contains(info.id)) {
        existingKey = info.id;
    } else {
        for (auto it = devices_.cbegin(); it != devices_.cend(); ++it) {
            const DeviceInfo current = it.value()->info();
            if (current.ipAddress == info.ipAddress && current.port == info.port) {
                existingKey = it.key();
                break;
            }
        }
    }

    if (!existingKey.isEmpty()) {
        auto* controller = devices_.take(existingKey);
        const bool remembered = rememberedIds_.remove(existingKey);
        controller->updateDiscovery(info, state);
        devices_.insert(info.stableId(), controller);
        if (remembered) {
            rememberedIds_.insert(info.stableId());
        }
        emit deviceUpdated(controller);
        return;
    }

    auto* controller = new DeviceController(info, state, this);
    devices_.insert(info.stableId(), controller);
    emit deviceAdded(controller);
}
