#include "DevicePage.h"

#include "../widgets/SegmentPreviewWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace {
QString statusName(YeelightConnection::Status status) {
    switch (status) {
    case YeelightConnection::Status::Disconnected: return QStringLiteral("Disconnected");
    case YeelightConnection::Status::Connecting: return QStringLiteral("Connecting");
    case YeelightConnection::Status::Connected: return QStringLiteral("Connected");
    case YeelightConnection::Status::Reconnecting: return QStringLiteral("Reconnecting");
    case YeelightConnection::Status::Error: return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}
}

DevicePage::DevicePage(DeviceManager* manager, QWidget* parent)
    : QWidget(parent)
    , manager_(manager)
    , idValue_(new QLabel(this))
    , addressValue_(new QLabel(this))
    , portValue_(new QLabel(this))
    , modelValue_(new QLabel(this))
    , firmwareValue_(new QLabel(this))
    , nameValue_(new QLabel(this))
    , methodsValue_(new QLabel(this))
    , statusValue_(new QLabel(this))
    , lastSeenValue_(new QLabel(this))
    , rememberedValue_(new QLabel(this))
    , segmentNotice_(new QLabel(this))
    , segmentPreview_(new SegmentPreviewWidget(this)) {
    methodsValue_->setWordWrap(true);
    methodsValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    segmentNotice_->setWordWrap(true);
    auto* form = new QFormLayout;
    form->addRow(tr("Device ID"), idValue_);
    form->addRow(tr("IP address"), addressValue_);
    form->addRow(tr("TCP port"), portValue_);
    form->addRow(tr("Model"), modelValue_);
    form->addRow(tr("Firmware"), firmwareValue_);
    form->addRow(tr("LAN name"), nameValue_);
    form->addRow(tr("Advertised methods"), methodsValue_);
    form->addRow(tr("Connection status"), statusValue_);
    form->addRow(tr("Last seen"), lastSeenValue_);
    form->addRow(tr("Remembered"), rememberedValue_);

    auto* refreshButton = new QPushButton(tr("Refresh state"), this);
    auto* reconnectButton = new QPushButton(tr("Reconnect"), this);
    auto* localNameButton = new QPushButton(tr("Rename locally"), this);
    auto* deviceNameButton = new QPushButton(tr("Rename on device"), this);
    auto* forgetButton = new QPushButton(tr("Forget device"), this);
    auto* diagnosticsButton = new QPushButton(tr("Copy diagnostics"), this);
    auto* manualButton = new QPushButton(tr("Open manual connection"), this);
    auto* buttons = new QHBoxLayout;
    for (auto* button : {
             refreshButton, reconnectButton, localNameButton, deviceNameButton,
             forgetButton, diagnosticsButton, manualButton
         }) {
        buttons->addWidget(button);
    }
    buttons->addStretch();

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(segmentNotice_);
    layout->addWidget(segmentPreview_);
    layout->addLayout(buttons);
    layout->addStretch();

    connect(refreshButton, &QPushButton::clicked, this, [this] {
        if (device_ != nullptr) {
            device_->refreshState();
        }
    });
    connect(reconnectButton, &QPushButton::clicked, this, [this] {
        if (device_ != nullptr) {
            device_->connectDevice();
        }
    });
    connect(localNameButton, &QPushButton::clicked, this, &DevicePage::renameLocal);
    connect(deviceNameButton, &QPushButton::clicked, this, &DevicePage::renameOnDevice);
    connect(forgetButton, &QPushButton::clicked, this, [this] {
        if (device_ != nullptr) {
            const QString id = device_->info().stableId();
            setDevice(nullptr);
            manager_->removeRememberedDevice(id);
        }
    });
    connect(diagnosticsButton, &QPushButton::clicked,
        this, &DevicePage::copyDiagnostics);
    connect(manualButton, &QPushButton::clicked,
        this, &DevicePage::addDeviceRequested);
    setDevice(nullptr);
}

void DevicePage::setDevice(DeviceController* device) {
    if (device_ != nullptr) {
        disconnect(device_, nullptr, this, nullptr);
    }
    device_ = device;
    setEnabled(device_ != nullptr);
    if (device_ != nullptr) {
        connect(device_, &DeviceController::infoChanged,
            this, &DevicePage::refresh);
        connect(device_, &DeviceController::stateChanged,
            this, &DevicePage::refresh);
        connect(device_, &DeviceController::connectionStatusChanged,
            this, &DevicePage::refresh);
    }
    refresh();
}

void DevicePage::refresh() {
    if (device_ == nullptr) {
        for (auto* label : {
                 idValue_, addressValue_, portValue_, modelValue_, firmwareValue_,
                 nameValue_, methodsValue_, statusValue_, lastSeenValue_,
                 rememberedValue_
             }) {
            label->clear();
        }
        segmentPreview_->hide();
        segmentNotice_->hide();
        return;
    }
    const DeviceInfo info = device_->info();
    const DeviceState state = device_->state();
    idValue_->setText(info.id.isEmpty() ? tr("Not advertised") : info.id);
    addressValue_->setText(info.ipAddress);
    portValue_->setText(QString::number(info.port));
    modelValue_->setText(info.model.isEmpty() ? tr("Unknown") : info.model);
    firmwareValue_->setText(
        info.firmwareVersion.isEmpty() ? tr("Unknown") : info.firmwareVersion
    );
    nameValue_->setText(info.name.isEmpty() ? tr("Unnamed") : info.name);
    QStringList methods;
    for (const auto& method : info.capabilities.methods) {
        methods.append(method);
    }
    methods.sort();
    methodsValue_->setText(methods.join(QStringLiteral(", ")));
    statusValue_->setText(statusName(device_->connectionStatus()));
    lastSeenValue_->setText(
        state.lastSeen.isValid() ? state.lastSeen.toLocalTime().toString(Qt::ISODate)
                                 : tr("Never")
    );
    const auto remembered = manager_->rememberedDevices();
    const bool isRemembered = std::any_of(
        remembered.cbegin(),
        remembered.cend(),
        [&info](const DeviceInfo& entry) { return entry.stableId() == info.stableId(); }
    );
    rememberedValue_->setText(isRemembered ? tr("Yes") : tr("No"));
    const bool segments = info.capabilities.supportsSegmentControl();
    segmentPreview_->setVisible(segments);
    segmentNotice_->setVisible(!segments);
    segmentNotice_->setText(
        tr("This device does not expose segment control through the detected LAN protocol.")
    );
}

void DevicePage::copyDiagnostics() {
    if (device_ == nullptr) {
        return;
    }
    const DeviceInfo info = device_->info();
    const QString diagnostics = QStringLiteral(
        "Device: %1\nAddress: %2:%3\nModel: %4\nFirmware: %5\nStatus: %6"
    ).arg(
        info.id,
        info.ipAddress,
        QString::number(info.port),
        info.model,
        info.firmwareVersion,
        statusName(device_->connectionStatus())
    );
    QApplication::clipboard()->setText(diagnostics);
}

void DevicePage::renameLocal() {
    if (device_ == nullptr) {
        return;
    }
    bool accepted = false;
    const QString name = QInputDialog::getText(
        this,
        tr("Rename locally"),
        tr("Local display name"),
        QLineEdit::Normal,
        device_->info().name,
        &accepted
    ).trimmed();
    if (accepted && !name.isEmpty()) {
        device_->setLocalName(name);
    }
}

void DevicePage::renameOnDevice() {
    if (device_ == nullptr) {
        return;
    }
    bool accepted = false;
    const QString name = QInputDialog::getText(
        this,
        tr("Rename on device"),
        tr("LAN device name"),
        QLineEdit::Normal,
        device_->info().name,
        &accepted
    ).trimmed();
    if (accepted) {
        device_->setDeviceName(name);
    }
}
