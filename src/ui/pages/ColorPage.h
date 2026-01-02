#pragma once

#include "../../core/device/DeviceController.h"

#include <QWidget>

#include <functional>

class ColorPreviewWidget;
class QLineEdit;
class QSlider;
class QSpinBox;
class QTimer;

class ColorPage final : public QWidget {
    Q_OBJECT

public:
    explicit ColorPage(QWidget* parent = nullptr);
    void setDevice(DeviceController* device);

private:
    void chooseColor();
    void applyColor(const QColor& color);
    void updateFromState();
    void updateCapabilities();
    void schedule(std::function<void()> action);
    void sendPending();

    DeviceController* device_ = nullptr;
    ColorPreviewWidget* preview_;
    QLineEdit* hexEdit_;
    QSlider* brightnessSlider_;
    QSlider* temperatureSlider_;
    QSlider* hueSlider_;
    QSlider* saturationSlider_;
    QSpinBox* durationSpin_;
    QTimer* throttleTimer_;
    std::function<void()> pendingAction_;
};

