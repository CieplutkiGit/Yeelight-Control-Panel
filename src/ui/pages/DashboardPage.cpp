#include "DashboardPage.h"

#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent)
    , powerStateLabel_(new QLabel(tr("Unknown"), this))
    , brightnessValueLabel_(new QLabel(tr("Unknown"), this))
    , colorModeLabel_(new QLabel(tr("Unknown"), this))
    , temperatureValueLabel_(new QLabel(tr("Unknown"), this))
    , lastSeenLabel_(new QLabel(tr("Never"), this))
    , connectionStatusLabel_(new QLabel(tr("Disconnected"), this))
    , refreshStateButton_(new QPushButton(tr("Refresh state"), this)) {
    powerStateLabel_->setObjectName(QStringLiteral("powerStateLabel"));
    brightnessValueLabel_->setObjectName(QStringLiteral("brightnessValueLabel"));
    colorModeLabel_->setObjectName(QStringLiteral("colorModeLabel"));
    temperatureValueLabel_->setObjectName(QStringLiteral("temperatureValueLabel"));
    lastSeenLabel_->setObjectName(QStringLiteral("lastSeenLabel"));
    connectionStatusLabel_->setObjectName(QStringLiteral("connectionStatusLabel"));
    refreshStateButton_->setObjectName(QStringLiteral("refreshStateButton"));

    auto* form = new QFormLayout;
    form->addRow(tr("Connection"), connectionStatusLabel_);
    form->addRow(tr("Power"), powerStateLabel_);
    form->addRow(tr("Brightness"), brightnessValueLabel_);
    form->addRow(tr("Color mode"), colorModeLabel_);
    form->addRow(tr("Temperature"), temperatureValueLabel_);
    form->addRow(tr("Last response"), lastSeenLabel_);
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(refreshStateButton_, 0, Qt::AlignLeft);
    layout->addStretch();
    connect(refreshStateButton_, &QPushButton::clicked, this, [this] {
        if (device_ != nullptr) {
            device_->refreshState();
        }
    });
}

void DashboardPage::setDevice(DeviceController* device) {
    if (device_ != nullptr) {
        disconnect(device_, nullptr, this, nullptr);
    }
    device_ = device;
    if (device_ != nullptr) {
        connect(device_, &DeviceController::stateChanged, this, &DashboardPage::refresh);
        connect(device_, &DeviceController::connectionStatusChanged,
            this, &DashboardPage::refresh);
    }
    refresh();
}

void DashboardPage::refresh() {
    if (device_ == nullptr) {
        setEnabled(false);
        return;
    }
    setEnabled(true);
    const DeviceState state = device_->state();
    powerStateLabel_->setText(
        state.power == PowerState::On
            ? tr("On")
            : (state.power == PowerState::Off ? tr("Off") : tr("Unknown"))
    );
    brightnessValueLabel_->setText(
        state.brightness < 0 ? tr("Unknown") : tr("%1%").arg(state.brightness)
    );
    temperatureValueLabel_->setText(
        state.colorTemperature < 0
            ? tr("Unknown")
            : tr("%1 K").arg(state.colorTemperature)
    );
    colorModeLabel_->setText(QString::number(static_cast<int>(state.colorMode)));
    lastSeenLabel_->setText(
        state.lastSeen.isValid()
            ? QLocale::system().toString(
                state.lastSeen.toLocalTime(),
                QLocale::ShortFormat
            )
            : tr("Never")
    );
    connectionStatusLabel_->setText(
        state.reachable ? tr("Online") : tr("Offline")
    );
}
