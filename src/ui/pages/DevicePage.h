#pragma once

#include "../../core/device/DeviceManager.h"

#include <QWidget>

class QLabel;
class SegmentPreviewWidget;

class DevicePage final : public QWidget {
    Q_OBJECT

public:
    explicit DevicePage(DeviceManager* manager, QWidget* parent = nullptr);
    void setDevice(DeviceController* device);

signals:
    void addDeviceRequested();

private:
    void refresh();
    void copyDiagnostics();
    void renameLocal();
    void renameOnDevice();

    DeviceManager* manager_;
    DeviceController* device_ = nullptr;
    QLabel* idValue_;
    QLabel* addressValue_;
    QLabel* portValue_;
    QLabel* modelValue_;
    QLabel* firmwareValue_;
    QLabel* nameValue_;
    QLabel* methodsValue_;
    QLabel* statusValue_;
    QLabel* lastSeenValue_;
    QLabel* rememberedValue_;
    QLabel* segmentNotice_;
    SegmentPreviewWidget* segmentPreview_;
};

