#pragma once

#include "../../core/device/DeviceController.h"

#include <QList>
#include <QWidget>

#include <functional>

class CardWidget;
class ColorPreviewWidget;
class QComboBox;
class QGridLayout;
class QLabel;
class QPushButton;
class QSlider;
class QTimer;

class DashboardPage final : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);
    void setDevice(DeviceController* device);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void arrangeCards();
    void applyColor(const QColor& color);
    void refresh();
    void updateCapabilities();
    void schedule(std::function<void()> action);
    void sendPending();

    DeviceController* device_ = nullptr;
    QGridLayout* grid_;
    QList<CardWidget*> cards_;
    int gridColumns_ = 0;
    QLabel* powerStateLabel_;
    QLabel* brightnessValueLabel_;
    QLabel* colorValueLabel_;
    QLabel* temperatureValueLabel_;
    QLabel* lastSeenLabel_;
    QLabel* connectionStatusLabel_;
    ColorPreviewWidget* colorPreview_;
    QSlider* brightnessSlider_;
    QSlider* temperatureSlider_;
    QPushButton* powerToggle_;
    QComboBox* transitionCombo_;
    QPushButton* refreshStateButton_;
    QTimer* throttleTimer_;
    std::function<void()> pendingAction_;
};
