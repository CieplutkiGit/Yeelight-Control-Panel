#include "DeviceListModel.h"

DeviceListModel::DeviceListModel(DeviceManager* manager, QObject* parent)
    : QAbstractListModel(parent)
    , manager_(manager)
    , devices_(manager->devices()) {
    connect(manager_, &DeviceManager::deviceAdded,
        this, &DeviceListModel::addController);
    connect(manager_, &DeviceManager::deviceUpdated,
        this, &DeviceListModel::refreshController);
    connect(manager_, &DeviceManager::deviceRemoved, this,
        [this](const QString& stableId) {
            for (int row = 0; row < static_cast<int>(devices_.size()); ++row) {
                if (devices_.at(row)->info().stableId() == stableId) {
                    beginRemoveRows({}, row, row);
                    devices_.removeAt(row);
                    endRemoveRows();
                    return;
                }
            }
        });
}

int DeviceListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(devices_.size());
}

QVariant DeviceListModel::data(const QModelIndex& index, int role) const {
    auto* controller = controllerAt(index.row());
    if (!index.isValid() || controller == nullptr) {
        return {};
    }
    const DeviceInfo info = controller->info();
    const DeviceState state = controller->state();
    const QString displayName = info.name.isEmpty()
        ? (info.model.isEmpty() ? info.ipAddress : info.model)
        : info.name;
    switch (role) {
    case Qt::DisplayRole:
        return QStringLiteral("%1\n%2 · %3")
            .arg(
                displayName,
                info.model.isEmpty() ? QStringLiteral("Yeelight") : info.model,
                info.ipAddress
            );
    case Qt::ToolTipRole:
        return QStringLiteral("%1:%2").arg(info.ipAddress).arg(info.port);
    case StableIdRole: return info.stableId();
    case ModelRole: return info.model;
    case AddressRole: return info.ipAddress;
    case OnlineRole: return state.reachable;
    case PowerRole: return static_cast<int>(state.power);
    case FavoriteRole: return false;
    case ControllerRole: return QVariant::fromValue(controller);
    default: return {};
    }
}

QHash<int, QByteArray> DeviceListModel::roleNames() const {
    return {
        {StableIdRole, "stableId"},
        {ModelRole, "model"},
        {AddressRole, "address"},
        {OnlineRole, "online"},
        {PowerRole, "power"},
        {FavoriteRole, "favorite"},
        {ControllerRole, "controller"}
    };
}

DeviceController* DeviceListModel::controllerAt(int row) const {
    return row >= 0 && row < static_cast<int>(devices_.size())
        ? devices_.at(row)
        : nullptr;
}

void DeviceListModel::addController(DeviceController* controller) {
    const int row = static_cast<int>(devices_.size());
    beginInsertRows({}, row, row);
    devices_.append(controller);
    endInsertRows();
    connect(controller, &DeviceController::infoChanged, this,
        [this, controller] { refreshController(controller); });
    connect(controller, &DeviceController::stateChanged, this,
        [this, controller] { refreshController(controller); });
}

void DeviceListModel::refreshController(DeviceController* controller) {
    const int row = devices_.indexOf(controller);
    if (row >= 0) {
        const QModelIndex changed = index(row);
        emit dataChanged(changed, changed);
    }
}
