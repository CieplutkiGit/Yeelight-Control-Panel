#pragma once

#include "../../core/device/DeviceController.h"

#include <QWidget>

class QLabel;
class QPushButton;

class DashboardPage final : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);
    void setDevice(DeviceController* device);

private:
    void refresh();
    DeviceController* device_ = nullptr;
    QLabel* powerStateLabel_;
    QLabel* brightnessValueLabel_;
    QLabel* colorModeLabel_;
    QLabel* temperatureValueLabel_;
    QLabel* lastSeenLabel_;
    QLabel* connectionStatusLabel_;
    QPushButton* refreshStateButton_;
};

