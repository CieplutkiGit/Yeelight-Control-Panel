#pragma once

#include "../core/device/DeviceManager.h"

#include <QMainWindow>

class DashboardPage;
class DeviceListModel;
class QLabel;
class QListView;
class QPushButton;
class QSortFilterProxyModel;
class QStackedWidget;
class QTabWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(DeviceManager* manager, QWidget* parent = nullptr);

signals:
    void themeRequested(const QString& theme);
    void windowClosing(const QByteArray& geometry);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QWidget* makeInformationalPage(const QString& title, const QString& text);
    void selectDevice(const QModelIndex& proxyIndex);
    void updateSelection(DeviceController* controller);
    void updateStatusText();
    void addManualDevice();
    void showAbout();

    DeviceManager* manager_;
    DeviceController* selectedDevice_ = nullptr;
    DeviceListModel* deviceModel_;
    QSortFilterProxyModel* proxyModel_;
    QListView* deviceListView_;
    QLabel* statusLabel_;
    QLabel* selectedNameLabel_;
    QLabel* selectedDetailsLabel_;
    QLabel* connectionBadge_;
    QPushButton* powerOnButton_;
    QPushButton* powerOffButton_;
    QStackedWidget* contentStack_;
    QTabWidget* tabs_;
    DashboardPage* dashboardPage_;
};

