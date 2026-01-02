#include "EffectsPage.h"

#include "../dialogs/EffectEditorDialog.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
EffectPreset makeEffect(
    const QString& name,
    std::initializer_list<EffectStep> steps,
    int repeatCount = 0
) {
    EffectPreset effect;
    effect.id = QUuid::createUuid();
    effect.name = name;
    effect.steps = QList<EffectStep>(steps);
    effect.repeatCount = repeatCount;
    return effect;
}
}

EffectsPage::EffectsPage(SettingsRepository* settings, QWidget* parent)
    : QWidget(parent)
    , settings_(settings)
    , list_(new QListWidget(this))
    , playButton_(new QPushButton(tr("Play"), this))
    , stopButton_(new QPushButton(tr("Stop"), this))
    , editButton_(new QPushButton(tr("Edit"), this))
    , deleteButton_(new QPushButton(tr("Delete"), this)) {
    effects_ = builtInEffects();
    builtInCount_ = static_cast<int>(effects_.size());
    if (settings_ != nullptr) {
        effects_.append(settings_->loadEffects());
    }
    auto* addButton = new QPushButton(tr("New effect"), this);
    auto* duplicateButton = new QPushButton(tr("Duplicate"), this);
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(playButton_);
    buttons->addWidget(stopButton_);
    buttons->addWidget(addButton);
    buttons->addWidget(duplicateButton);
    buttons->addWidget(editButton_);
    buttons->addWidget(deleteButton_);
    buttons->addStretch();
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(list_, 1);
    layout->addLayout(buttons);
    connect(playButton_, &QPushButton::clicked, this, &EffectsPage::playSelected);
    connect(stopButton_, &QPushButton::clicked, this, [this] {
        if (device_ != nullptr) {
            device_->stopEffect();
        }
    });
    connect(addButton, &QPushButton::clicked, this, &EffectsPage::createEffect);
    connect(duplicateButton, &QPushButton::clicked,
        this, &EffectsPage::duplicateSelected);
    connect(editButton_, &QPushButton::clicked, this, &EffectsPage::editSelected);
    connect(deleteButton_, &QPushButton::clicked, this, &EffectsPage::deleteSelected);
    connect(list_, &QListWidget::currentRowChanged, this, [this](int row) {
        editButton_->setEnabled(row >= builtInCount_);
        deleteButton_->setEnabled(row >= builtInCount_);
    });
    rebuild();
    setDevice(nullptr);
}

void EffectsPage::setDevice(DeviceController* device) {
    device_ = device;
    const bool supported = device_ != nullptr
        && device_->info().capabilities.supportsColorFlow();
    playButton_->setEnabled(supported);
    stopButton_->setEnabled(supported);
    list_->setToolTip(
        supported
            ? QString()
            : tr("The selected device does not advertise color-flow support.")
    );
}

QList<EffectPreset> EffectsPage::builtInEffects() {
    using Mode = EffectStep::Mode;
    return {
        makeEffect(QStringLiteral("Sunrise"), {
            {120000, Mode::ColorTemperature, 1700, 1},
            {120000, Mode::ColorTemperature, 3500, 45},
            {120000, Mode::ColorTemperature, 5000, 100}
        }, 1),
        makeEffect(QStringLiteral("Sunset"), {
            {90000, Mode::ColorTemperature, 4000, 70},
            {90000, Mode::ColorTemperature, 2200, 20},
            {30000, Mode::Rgb, 0xff4500, 5}
        }, 1),
        makeEffect(QStringLiteral("Relax"), {
            {2000, Mode::ColorTemperature, 2700, 35},
            {2000, Mode::ColorTemperature, 2400, 25}
        }),
        makeEffect(QStringLiteral("Reading"), {
            {1000, Mode::ColorTemperature, 4200, 90}
        }, 1),
        makeEffect(QStringLiteral("Night light"), {
            {1000, Mode::Rgb, 0xff5a1f, 5}
        }, 1),
        makeEffect(QStringLiteral("Color cycle"), {
            {1500, Mode::Rgb, 0xff0000, 100},
            {1500, Mode::Rgb, 0x00ff00, 100},
            {1500, Mode::Rgb, 0x0000ff, 100}
        }),
        makeEffect(QStringLiteral("Police lights"), {
            {250, Mode::Rgb, 0xff0000, 100},
            {250, Mode::Rgb, 0x0000ff, 100}
        }),
        makeEffect(QStringLiteral("Slow rainbow"), {
            {4000, Mode::Rgb, 0xff0000, 80},
            {4000, Mode::Rgb, 0xffff00, 80},
            {4000, Mode::Rgb, 0x00ff00, 80},
            {4000, Mode::Rgb, 0x00ffff, 80},
            {4000, Mode::Rgb, 0x0000ff, 80},
            {4000, Mode::Rgb, 0xff00ff, 80}
        })
    };
}

void EffectsPage::rebuild() {
    const int previous = list_->currentRow();
    list_->clear();
    for (int index = 0; index < static_cast<int>(effects_.size()); ++index) {
        const QString suffix = index < builtInCount_ ? tr(" · Built in") : tr(" · Custom");
        list_->addItem(effects_.at(index).name + suffix);
    }
    if (!effects_.isEmpty()) {
        list_->setCurrentRow(qBound(
            0,
            previous,
            static_cast<int>(effects_.size()) - 1
        ));
    }
}

void EffectsPage::playSelected() {
    const int index = selectedIndex();
    if (device_ != nullptr && index >= 0) {
        device_->startEffect(effects_.at(index));
    }
}

void EffectsPage::createEffect() {
    EffectEditorDialog dialog(
        settings_ != nullptr
            && settings_->value(QStringLiteral("settings/developerMode"), false).toBool(),
        this
    );
    if (dialog.exec() == QDialog::Accepted) {
        effects_.append(dialog.effect());
        saveCustomEffects();
        rebuild();
        list_->setCurrentRow(static_cast<int>(effects_.size()) - 1);
    }
}

void EffectsPage::duplicateSelected() {
    const int index = selectedIndex();
    if (index < 0) {
        return;
    }
    EffectPreset copy = effects_.at(index);
    copy.id = QUuid::createUuid();
    copy.name += tr(" copy");
    effects_.append(copy);
    saveCustomEffects();
    rebuild();
    list_->setCurrentRow(static_cast<int>(effects_.size()) - 1);
}

void EffectsPage::editSelected() {
    const int index = selectedIndex();
    if (index < builtInCount_) {
        return;
    }
    EffectEditorDialog dialog(
        settings_ != nullptr
            && settings_->value(QStringLiteral("settings/developerMode"), false).toBool(),
        this
    );
    dialog.setEffect(effects_.at(index));
    if (dialog.exec() == QDialog::Accepted) {
        effects_[index] = dialog.effect();
        saveCustomEffects();
        rebuild();
        list_->setCurrentRow(index);
    }
}

void EffectsPage::deleteSelected() {
    const int index = selectedIndex();
    if (index < builtInCount_) {
        return;
    }
    if (QMessageBox::question(
            this,
            tr("Delete effect"),
            tr("Delete the selected custom effect?")
        ) != QMessageBox::Yes) {
        return;
    }
    effects_.removeAt(index);
    saveCustomEffects();
    rebuild();
}

void EffectsPage::saveCustomEffects() {
    if (settings_ != nullptr) {
        settings_->saveEffects(effects_.mid(builtInCount_));
        settings_->sync();
    }
}

int EffectsPage::selectedIndex() const {
    const int row = list_->currentRow();
    return row >= 0 && row < static_cast<int>(effects_.size()) ? row : -1;
}
