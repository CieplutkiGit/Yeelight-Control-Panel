#include "ColorPage.h"

#include "../widgets/ColorPreviewWidget.h"

#include <QColorDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

namespace {
QColor colorFromRgbInteger(int rgb) {
    return rgb < 0
        ? QColor()
        : QColor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
}
}

ColorPage::ColorPage(QWidget* parent)
    : QWidget(parent)
    , preview_(new ColorPreviewWidget(this))
    , hexEdit_(new QLineEdit(this))
    , brightnessSlider_(new QSlider(Qt::Horizontal, this))
    , temperatureSlider_(new QSlider(Qt::Horizontal, this))
    , hueSlider_(new QSlider(Qt::Horizontal, this))
    , saturationSlider_(new QSlider(Qt::Horizontal, this))
    , durationSpin_(new QSpinBox(this))
    , throttleTimer_(new QTimer(this)) {
    preview_->setObjectName(QStringLiteral("colorPreview"));
    hexEdit_->setObjectName(QStringLiteral("hexColorEdit"));
    hexEdit_->setPlaceholderText(QStringLiteral("#RRGGBB"));
    brightnessSlider_->setObjectName(QStringLiteral("brightnessSlider"));
    brightnessSlider_->setRange(1, 100);
    temperatureSlider_->setObjectName(QStringLiteral("temperatureSlider"));
    temperatureSlider_->setRange(1700, 6500);
    hueSlider_->setObjectName(QStringLiteral("hueSlider"));
    hueSlider_->setRange(0, 359);
    saturationSlider_->setObjectName(QStringLiteral("saturationSlider"));
    saturationSlider_->setRange(0, 100);
    durationSpin_->setObjectName(QStringLiteral("transitionDurationSpin"));
    durationSpin_->setRange(30, 5000);
    durationSpin_->setSuffix(tr(" ms"));
    durationSpin_->setValue(300);
    throttleTimer_->setSingleShot(true);
    throttleTimer_->setInterval(100);

    auto* pickerLayout = new QVBoxLayout;
    pickerLayout->addWidget(preview_, 0, Qt::AlignHCenter);
    pickerLayout->addWidget(hexEdit_);
    auto* swatches = new QHBoxLayout;
    const QList<QPair<QString, QColor>> presets{
        {tr("Warm"), QColor(QStringLiteral("#ffd6a1"))},
        {tr("Neutral"), QColor(QStringLiteral("#fff4df"))},
        {tr("Cool"), QColor(QStringLiteral("#dcecff"))},
        {tr("Red"), Qt::red},
        {tr("Green"), Qt::green},
        {tr("Blue"), Qt::blue}
    };
    for (const auto& preset : presets) {
        auto* button = new QPushButton(preset.first, this);
        button->setProperty("swatchColor", preset.second.name());
        connect(button, &QPushButton::clicked, this,
            [this, color = preset.second] { applyColor(color); });
        swatches->addWidget(button);
    }
    pickerLayout->addLayout(swatches);

    auto* form = new QFormLayout;
    form->addRow(tr("Brightness"), brightnessSlider_);
    form->addRow(tr("White temperature"), temperatureSlider_);
    form->addRow(tr("Hue"), hueSlider_);
    form->addRow(tr("Saturation"), saturationSlider_);
    form->addRow(tr("Transition"), durationSpin_);
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(pickerLayout);
    layout->addLayout(form);
    layout->addStretch();

    connect(preview_, &ColorPreviewWidget::clicked, this, &ColorPage::chooseColor);
    connect(hexEdit_, &QLineEdit::editingFinished, this, [this] {
        const QColor color(hexEdit_->text());
        if (color.isValid()) {
            applyColor(color);
        }
    });
    connect(brightnessSlider_, &QSlider::valueChanged, this, [this](int value) {
        schedule([this, value] {
            if (device_ != nullptr) {
                device_->setBrightness(value);
            }
        });
    });
    connect(temperatureSlider_, &QSlider::valueChanged, this, [this](int value) {
        schedule([this, value] {
            if (device_ != nullptr) {
                device_->setColorTemperature(value);
            }
        });
    });
    const auto scheduleHsv = [this] {
        const int hue = hueSlider_->value();
        const int saturation = saturationSlider_->value();
        schedule([this, hue, saturation] {
            if (device_ != nullptr) {
                device_->setHsv(hue, saturation);
            }
        });
    };
    connect(hueSlider_, &QSlider::valueChanged, this, scheduleHsv);
    connect(saturationSlider_, &QSlider::valueChanged, this, scheduleHsv);
    for (auto* slider : {brightnessSlider_, temperatureSlider_, hueSlider_, saturationSlider_}) {
        connect(slider, &QSlider::sliderReleased, this, &ColorPage::sendPending);
    }
    connect(durationSpin_, &QSpinBox::valueChanged, this, [this](int value) {
        if (device_ != nullptr) {
            device_->setTransitionDuration(value);
        }
    });
    connect(throttleTimer_, &QTimer::timeout, this, &ColorPage::sendPending);
    setEnabled(false);
}

void ColorPage::setDevice(DeviceController* device) {
    if (device_ != nullptr) {
        disconnect(device_, nullptr, this, nullptr);
    }
    device_ = device;
    setEnabled(device_ != nullptr);
    if (device_ != nullptr) {
        connect(device_, &DeviceController::stateChanged,
            this, &ColorPage::updateFromState);
        connect(device_, &DeviceController::infoChanged,
            this, &ColorPage::updateCapabilities);
        device_->setTransitionDuration(durationSpin_->value());
    }
    updateCapabilities();
    updateFromState();
}

void ColorPage::chooseColor() {
    const QColor color = QColorDialog::getColor(preview_->color(), this, tr("Choose color"));
    if (color.isValid()) {
        applyColor(color);
    }
}

void ColorPage::applyColor(const QColor& color) {
    preview_->setColor(color);
    {
        const QSignalBlocker blocker(hexEdit_);
        hexEdit_->setText(color.name(QColor::HexRgb).toUpper());
    }
    schedule([this, color] {
        if (device_ != nullptr) {
            device_->setRgb(color);
        }
    });
}

void ColorPage::updateFromState() {
    if (device_ == nullptr) {
        return;
    }
    const DeviceState state = device_->state();
    const QColor color = colorFromRgbInteger(state.rgb);
    if (color.isValid()) {
        preview_->setColor(color);
        const QSignalBlocker blocker(hexEdit_);
        hexEdit_->setText(color.name(QColor::HexRgb).toUpper());
    }
    const QSignalBlocker brightnessBlocker(brightnessSlider_);
    const QSignalBlocker temperatureBlocker(temperatureSlider_);
    const QSignalBlocker hueBlocker(hueSlider_);
    const QSignalBlocker saturationBlocker(saturationSlider_);
    if (state.brightness >= 1) {
        brightnessSlider_->setValue(state.brightness);
    }
    if (state.colorTemperature >= 1700) {
        temperatureSlider_->setValue(state.colorTemperature);
    }
    if (state.hue >= 0) {
        hueSlider_->setValue(state.hue);
    }
    if (state.saturation >= 0) {
        saturationSlider_->setValue(state.saturation);
    }
}

void ColorPage::updateCapabilities() {
    if (device_ == nullptr) {
        return;
    }
    const DeviceCapabilities capabilities = device_->info().capabilities;
    preview_->setEnabled(capabilities.supportsRgb());
    hexEdit_->setEnabled(capabilities.supportsRgb());
    for (auto* button : findChildren<QPushButton*>()) {
        if (button->property("swatchColor").isValid()) {
            button->setEnabled(capabilities.supportsRgb());
        }
    }
    brightnessSlider_->setEnabled(capabilities.supportsBrightness());
    temperatureSlider_->setEnabled(capabilities.supportsColorTemperature());
    hueSlider_->setEnabled(capabilities.supportsHsv());
    saturationSlider_->setEnabled(capabilities.supportsHsv());
}

void ColorPage::schedule(std::function<void()> action) {
    pendingAction_ = std::move(action);
    if (!throttleTimer_->isActive()) {
        throttleTimer_->start();
    }
}

void ColorPage::sendPending() {
    throttleTimer_->stop();
    if (pendingAction_) {
        auto action = std::move(pendingAction_);
        pendingAction_ = {};
        action();
    }
}
