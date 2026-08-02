#include "DashboardPage.h"

#include "../widgets/CardWidget.h"
#include "../widgets/ColorPreviewWidget.h"
#include "../widgets/PowerToggle.h"

#include <QColorDialog>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

namespace {
QColor colorFromRgbInteger(int rgb) {
    return rgb < 0
        ? QColor()
        : QColor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
}

QString powerText(PowerState power) {
    return power == PowerState::On
        ? QObject::tr("On")
        : (power == PowerState::Off ? QObject::tr("Off") : QObject::tr("Unknown"));
}
}

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent)
    , grid_(new QGridLayout)
    , powerStateLabel_(new QLabel(tr("Unknown"), this))
    , brightnessValueLabel_(new QLabel(tr("Unknown"), this))
    , colorValueLabel_(new QLabel(tr("Unknown"), this))
    , temperatureValueLabel_(new QLabel(tr("Unknown"), this))
    , lastSeenLabel_(new QLabel(tr("Never"), this))
    , connectionStatusLabel_(new QLabel(tr("Disconnected"), this))
    , colorPreview_(new ColorPreviewWidget(this))
    , brightnessSlider_(new QSlider(Qt::Horizontal, this))
    , temperatureSlider_(new QSlider(Qt::Horizontal, this))
    , powerToggle_(new PowerToggle(this))
    , transitionCombo_(new QComboBox(this))
    , refreshStateButton_(new QPushButton(tr("Refresh state"), this))
    , throttleTimer_(new QTimer(this)) {
    setObjectName(QStringLiteral("dashboardPage"));

    colorPreview_->setObjectName(QStringLiteral("dashboardColorPreview"));
    colorPreview_->setToolTip(tr("Choose a color"));
    brightnessSlider_->setObjectName(QStringLiteral("dashboardBrightnessSlider"));
    brightnessSlider_->setRange(1, 100);
    temperatureSlider_->setObjectName(QStringLiteral("dashboardTemperatureSlider"));
    temperatureSlider_->setRange(1700, 6500);
    powerToggle_->setObjectName(QStringLiteral("powerToggle"));
    transitionCombo_->setObjectName(QStringLiteral("transitionCombo"));
    transitionCombo_->addItem(tr("0.3 s"), 300);
    transitionCombo_->addItem(tr("1.0 s"), 1000);
    transitionCombo_->addItem(tr("2.0 s"), 2000);
    transitionCombo_->addItem(tr("5.0 s"), 5000);
    refreshStateButton_->setObjectName(QStringLiteral("refreshStateButton"));
    powerStateLabel_->setObjectName(QStringLiteral("powerStateLabel"));
    brightnessValueLabel_->setObjectName(QStringLiteral("brightnessValueLabel"));
    colorValueLabel_->setObjectName(QStringLiteral("colorValueLabel"));
    temperatureValueLabel_->setObjectName(QStringLiteral("temperatureValueLabel"));
    lastSeenLabel_->setObjectName(QStringLiteral("lastSeenLabel"));
    connectionStatusLabel_->setObjectName(QStringLiteral("connectionStatusLabel"));

    auto* brightnessCard = new CardWidget(
        tr("Brightness"), QStringLiteral(":/icons/brightness.svg"), this
    );
    auto* brightnessValueRow = new QHBoxLayout;
    brightnessValueRow->addWidget(new QLabel(tr("Level"), brightnessCard));
    brightnessValueRow->addStretch();
    brightnessValueRow->addWidget(brightnessValueLabel_);
    brightnessCard->contentLayout()->addLayout(brightnessValueRow);
    brightnessCard->contentLayout()->addWidget(brightnessSlider_);

    auto* colorCard = new CardWidget(
        tr("Color"), QStringLiteral(":/icons/color.svg"), this
    );
    auto* colorLayout = new QHBoxLayout;
    colorPreview_->setMinimumSize(188, 188);
    colorPreview_->setMaximumSize(220, 220);
    colorLayout->addWidget(colorPreview_, 0, Qt::AlignCenter);
    auto* colorDetails = new QVBoxLayout;
    colorDetails->addWidget(new QLabel(tr("Selected color"), colorCard));
    colorDetails->addWidget(colorValueLabel_);
    colorDetails->addStretch();
    colorLayout->addLayout(colorDetails, 1);
    colorCard->contentLayout()->addLayout(colorLayout);

    auto* temperatureCard = new CardWidget(
        tr("White temperature"), QStringLiteral(":/icons/temperature.svg"), this
    );
    auto* temperatureValueRow = new QHBoxLayout;
    temperatureValueRow->addWidget(new QLabel(tr("Warm"), temperatureCard));
    temperatureValueRow->addStretch();
    temperatureValueRow->addWidget(temperatureValueLabel_);
    temperatureValueRow->addWidget(new QLabel(tr("Cool"), temperatureCard));
    temperatureCard->contentLayout()->addLayout(temperatureValueRow);
    temperatureCard->contentLayout()->addWidget(temperatureSlider_);

    auto* powerCard = new CardWidget(
        tr("Power"), QStringLiteral(":/icons/power.svg"), this
    );
    auto* powerLayout = new QHBoxLayout;
    powerLayout->addWidget(powerToggle_);
    powerLayout->addStretch();
    powerLayout->addWidget(powerStateLabel_);
    powerCard->contentLayout()->addLayout(powerLayout);
    powerCard->contentLayout()->addWidget(connectionStatusLabel_);

    auto* transitionCard = new CardWidget(
        tr("Transition"), QStringLiteral(":/icons/timer.svg"), this
    );
    transitionCard->contentLayout()->addWidget(new QLabel(tr("Animation duration"), transitionCard));
    transitionCard->contentLayout()->addWidget(transitionCombo_);
    transitionCard->contentLayout()->addWidget(refreshStateButton_);

    cards_ = {brightnessCard, colorCard, temperatureCard, powerCard, transitionCard};
    brightnessCard->setMinimumHeight(166);
    colorCard->setMinimumHeight(350);
    temperatureCard->setMinimumHeight(166);
    powerCard->setMinimumHeight(132);
    transitionCard->setMinimumHeight(132);

    grid_->setHorizontalSpacing(14);
    grid_->setVerticalSpacing(14);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(grid_);
    layout->addWidget(lastSeenLabel_);
    lastSeenLabel_->setVisible(false);
    arrangeCards();

    throttleTimer_->setSingleShot(true);
    throttleTimer_->setInterval(100);
    connect(refreshStateButton_, &QPushButton::clicked, this, [this] {
        if (device_ != nullptr) {
            device_->refreshState();
        }
    });
    connect(powerToggle_, &QAbstractButton::toggled, this, [this](bool on) {
        if (device_ != nullptr) {
            device_->setPower(on);
        }
    });
    connect(colorPreview_, &ColorPreviewWidget::clicked, this, [this] {
        const QColor color = QColorDialog::getColor(colorPreview_->color(), this,
                                                     tr("Choose color"));
        if (color.isValid()) {
            applyColor(color);
        }
    });
    connect(brightnessSlider_, &QSlider::valueChanged, this, [this](int value) {
        brightnessValueLabel_->setText(tr("%1%").arg(value));
        schedule([this, value] {
            if (device_ != nullptr) {
                device_->setBrightness(value);
            }
        });
    });
    connect(temperatureSlider_, &QSlider::valueChanged, this, [this](int value) {
        temperatureValueLabel_->setText(tr("%1 K").arg(value));
        schedule([this, value] {
            if (device_ != nullptr) {
                device_->setColorTemperature(value);
            }
        });
    });
    connect(brightnessSlider_, &QSlider::sliderReleased,
        this, &DashboardPage::sendPending);
    connect(temperatureSlider_, &QSlider::sliderReleased,
        this, &DashboardPage::sendPending);
    connect(transitionCombo_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (device_ != nullptr && index >= 0) {
            device_->setTransitionDuration(transitionCombo_->itemData(index).toInt());
        }
    });
    connect(throttleTimer_, &QTimer::timeout, this, &DashboardPage::sendPending);
    setEnabled(false);
}

void DashboardPage::setDevice(DeviceController* device) {
    if (device_ != nullptr) {
        disconnect(device_, nullptr, this, nullptr);
    }
    device_ = device;
    setEnabled(device_ != nullptr);
    if (device_ != nullptr) {
        connect(device_, &DeviceController::stateChanged,
            this, &DashboardPage::refresh);
        connect(device_, &DeviceController::connectionStatusChanged,
            this, &DashboardPage::refresh);
        device_->setTransitionDuration(transitionCombo_->currentData().toInt());
    }
    updateCapabilities();
    refresh();
}

void DashboardPage::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    arrangeCards();
}

void DashboardPage::arrangeCards() {
    const int columns = width() >= 760 ? 2 : 1;
    if (columns == gridColumns_ && grid_->count() == cards_.size()) {
        return;
    }
    gridColumns_ = columns;
    while (grid_->count() > 0) {
        grid_->takeAt(0);
    }
    for (int column = 0; column < 2; ++column) {
        grid_->setColumnStretch(column, column < columns ? 1 : 0);
    }
    if (columns == 2) {
        grid_->addWidget(cards_.at(0), 0, 0);
        grid_->addWidget(cards_.at(1), 0, 1, 2, 1);
        grid_->addWidget(cards_.at(2), 1, 0);
        grid_->addWidget(cards_.at(3), 2, 0);
        grid_->addWidget(cards_.at(4), 2, 1);
    } else {
        for (int index = 0; index < cards_.size(); ++index) {
            grid_->addWidget(cards_.at(index), index, 0);
        }
    }
}

void DashboardPage::applyColor(const QColor& color) {
    colorPreview_->setColor(color);
    colorValueLabel_->setText(color.name(QColor::HexRgb).toUpper());
    if (device_ != nullptr) {
        device_->setRgb(color);
    }
}

void DashboardPage::refresh() {
    if (device_ == nullptr) {
        powerStateLabel_->setText(tr("Unknown"));
        brightnessValueLabel_->setText(tr("Unknown"));
        colorValueLabel_->setText(tr("Unknown"));
        temperatureValueLabel_->setText(tr("Unknown"));
        connectionStatusLabel_->setText(tr("Disconnected"));
        lastSeenLabel_->setText(tr("Never"));
        return;
    }
    const DeviceState state = device_->state();
    const QSignalBlocker powerBlocker(powerToggle_);
    powerToggle_->setChecked(state.power == PowerState::On);
    powerStateLabel_->setText(powerText(state.power));
    connectionStatusLabel_->setText(state.reachable ? tr("Connected") : tr("Disconnected"));
    lastSeenLabel_->setText(
        state.lastSeen.isValid()
            ? QLocale::system().toString(state.lastSeen.toLocalTime(), QLocale::ShortFormat)
            : tr("Never")
    );

    const QSignalBlocker brightnessBlocker(brightnessSlider_);
    const QSignalBlocker temperatureBlocker(temperatureSlider_);
    if (state.brightness >= brightnessSlider_->minimum()) {
        brightnessSlider_->setValue(state.brightness);
        brightnessValueLabel_->setText(tr("%1%").arg(state.brightness));
    }
    if (state.colorTemperature >= temperatureSlider_->minimum()) {
        temperatureSlider_->setValue(state.colorTemperature);
        temperatureValueLabel_->setText(tr("%1 K").arg(state.colorTemperature));
    }
    const QColor color = colorFromRgbInteger(state.rgb);
    if (color.isValid()) {
        colorPreview_->setColor(color);
        colorValueLabel_->setText(color.name(QColor::HexRgb).toUpper());
    }
}

void DashboardPage::updateCapabilities() {
    if (device_ == nullptr) {
        return;
    }
    const DeviceCapabilities capabilities = device_->info().capabilities;
    powerToggle_->setEnabled(capabilities.supportsPower());
    brightnessSlider_->setEnabled(capabilities.supportsBrightness());
    temperatureSlider_->setEnabled(capabilities.supportsColorTemperature());
    colorPreview_->setEnabled(capabilities.supportsRgb());
}

void DashboardPage::schedule(std::function<void()> action) {
    pendingAction_ = std::move(action);
    if (!throttleTimer_->isActive()) {
        throttleTimer_->start();
    }
}

void DashboardPage::sendPending() {
    throttleTimer_->stop();
    if (pendingAction_) {
        auto action = std::move(pendingAction_);
        pendingAction_ = {};
        action();
    }
}
