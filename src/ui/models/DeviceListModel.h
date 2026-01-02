#pragma once

#include "../../core/device/DeviceManager.h"

#include <QAbstractListModel>
#include <QList>

class DeviceListModel final : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        StableIdRole = Qt::UserRole + 1,
        ModelRole,
        AddressRole,
        OnlineRole,
        PowerRole,
        FavoriteRole,
        ControllerRole
    };

    explicit DeviceListModel(DeviceManager* manager, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    DeviceController* controllerAt(int row) const;

private:
    void addController(DeviceController* controller);
    void refreshController(DeviceController* controller);

    DeviceManager* manager_;
    QList<DeviceController*> devices_;
};

