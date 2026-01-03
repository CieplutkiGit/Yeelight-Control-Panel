#pragma once

#include "../core/device/DeviceManager.h"

#include <QMainWindow>

class DashboardPage;
class ColorPage;
class ConnectionBadge;
class EffectsPage;
class AutomationsPage;
class DevicePage;
class LogsPage;
class AutomationEngine;
class SettingsRepository;
class DeviceListModel;
class QLabel;
class QListView;
class QPushButton;
class QSortFilterProxyModel;
class QStackedWidget;
class QTabWidget;
class QSplitter;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(
        DeviceManager* manager,
        SettingsRepository* settings,
        AutomationEngine* automations,
        QWidget* parent = nullptr
    );

signals:
    void themeRequested(const QString& theme);
    void windowClosing(const QByteArray& geometry);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void selectDevice(const QModelIndex& proxyIndex);
    void updateSelection(DeviceController* controller);
    void updateStatusText();
    void addManualDevice();
    void showAbout();

    DeviceManager* manager_;
    SettingsRepository* settings_;
    DeviceController* selectedDevice_ = nullptr;
    DeviceListModel* deviceModel_;
    QSortFilterProxyModel* proxyModel_;
    QListView* deviceListView_;
    QLabel* statusLabel_;
    QLabel* selectedNameLabel_;
    QLabel* selectedDetailsLabel_;
    ConnectionBadge* connectionBadge_;
    QPushButton* powerOnButton_;
    QPushButton* powerOffButton_;
    QPushButton* favoriteButton_;
    QStackedWidget* contentStack_;
    QTabWidget* tabs_;
    QSplitter* splitter_;
    DashboardPage* dashboardPage_;
    ColorPage* colorPage_;
    EffectsPage* effectsPage_;
    AutomationsPage* automationsPage_;
    DevicePage* devicePage_;
    LogsPage* logsPage_;
};
