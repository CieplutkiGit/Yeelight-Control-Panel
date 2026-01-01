#pragma once

#include "../model/DeviceInfo.h"
#include "../model/DeviceState.h"

#include <QObject>
#include <QTimer>
#include <QUdpSocket>

class DiscoveryService final : public QObject {
    Q_OBJECT

public:
    explicit DiscoveryService(QObject* parent = nullptr);

    void start();
    void stop();
    [[nodiscard]] bool isDiscovering() const;

signals:
    void deviceDiscovered(const DeviceInfo& device, const DeviceState& state);
    void discoveryError(const QString& message);
    void discoveryFinished();

private slots:
    void broadcast();
    void readPendingDatagrams();

private:
    QUdpSocket socket_;
    QTimer sessionTimer_;
    bool discovering_ = false;
};

Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(DeviceState)

