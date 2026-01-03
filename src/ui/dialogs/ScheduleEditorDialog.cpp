#include "ScheduleEditorDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimeEdit>
#include <QVBoxLayout>

ScheduleEditorDialog::ScheduleEditorDialog(
    DeviceManager* devices,
    QWidget* parent
)
    : QDialog(parent)
    , id_(QUuid::createUuid())
    , nameEdit_(new QLineEdit(this))
    , deviceCombo_(new QComboBox(this))
    , actionCombo_(new QComboBox(this))
    , valueEdit_(new QLineEdit(this))
    , timeEdit_(new QTimeEdit(this))
    , enabledCheck_(new QCheckBox(tr("Enabled"), this))
    , errorLabel_(new QLabel(this))
    , buttons_(new QDialogButtonBox(
          QDialogButtonBox::Save | QDialogButtonBox::Cancel,
          this
      )) {
    setWindowTitle(tr("Schedule editor"));
    setMinimumWidth(520);
    for (auto* controller : devices->devices()) {
        const DeviceInfo info = controller->info();
        deviceCombo_->addItem(
            info.name.isEmpty() ? info.ipAddress : info.name,
            info.stableId()
        );
    }
    actionCombo_->addItem(tr("Turn on"), static_cast<int>(ScheduledActionType::PowerOn));
    actionCombo_->addItem(tr("Turn off"), static_cast<int>(ScheduledActionType::PowerOff));
    actionCombo_->addItem(tr("Toggle"), static_cast<int>(ScheduledActionType::Toggle));
    actionCombo_->addItem(
        tr("Set brightness"),
        static_cast<int>(ScheduledActionType::SetBrightness)
    );
    actionCombo_->addItem(
        tr("Apply preset"),
        static_cast<int>(ScheduledActionType::ApplyPreset)
    );
    valueEdit_->setPlaceholderText(tr("Brightness 1–100 or preset UUID"));
    timeEdit_->setDisplayFormat(QStringLiteral("HH:mm"));
    enabledCheck_->setChecked(true);
    errorLabel_->setObjectName(QStringLiteral("validationErrorLabel"));

    auto* days = new QHBoxLayout;
    const QStringList dayNames{
        tr("Mon"), tr("Tue"), tr("Wed"), tr("Thu"),
        tr("Fri"), tr("Sat"), tr("Sun")
    };
    for (const auto& name : dayNames) {
        auto* check = new QCheckBox(name, this);
        dayChecks_.append(check);
        days->addWidget(check);
        connect(check, &QCheckBox::toggled, this, &ScheduleEditorDialog::validate);
    }
    auto* form = new QFormLayout;
    form->addRow(tr("Name"), nameEdit_);
    form->addRow(tr("Device"), deviceCombo_);
    form->addRow(tr("Action"), actionCombo_);
    form->addRow(tr("Value"), valueEdit_);
    form->addRow(tr("Time"), timeEdit_);
    form->addRow(tr("Days"), days);
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(enabledCheck_);
    layout->addWidget(errorLabel_);
    layout->addWidget(buttons_);

    const auto updateValueField = [this] {
        const auto type = static_cast<ScheduledActionType>(
            actionCombo_->currentData().toInt()
        );
        const bool needsValue = type == ScheduledActionType::SetBrightness
            || type == ScheduledActionType::ApplyPreset;
        valueEdit_->setEnabled(needsValue);
        validate();
    };
    connect(actionCombo_, &QComboBox::currentIndexChanged,
        this, updateValueField);
    connect(nameEdit_, &QLineEdit::textChanged, this, &ScheduleEditorDialog::validate);
    connect(deviceCombo_, &QComboBox::currentIndexChanged,
        this, &ScheduleEditorDialog::validate);
    connect(valueEdit_, &QLineEdit::textChanged, this, &ScheduleEditorDialog::validate);
    connect(buttons_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    updateValueField();
}

void ScheduleEditorDialog::setSchedule(const ScheduledAction& schedule) {
    id_ = schedule.id.isNull() ? QUuid::createUuid() : schedule.id;
    nameEdit_->setText(schedule.name);
    deviceCombo_->setCurrentIndex(deviceCombo_->findData(schedule.deviceId));
    actionCombo_->setCurrentIndex(actionCombo_->findData(
        static_cast<int>(schedule.type)
    ));
    valueEdit_->setText(schedule.value.toString());
    timeEdit_->setTime(schedule.time);
    for (int index = 0; index < dayChecks_.size(); ++index) {
        dayChecks_.at(index)->setChecked(schedule.days.contains(
            static_cast<Qt::DayOfWeek>(index + 1)
        ));
    }
    enabledCheck_->setChecked(schedule.enabled);
    lastExecuted_ = schedule.lastExecuted;
    validate();
}

ScheduledAction ScheduleEditorDialog::schedule() const {
    ScheduledAction result;
    result.id = id_;
    result.name = nameEdit_->text().trimmed();
    result.deviceId = deviceCombo_->currentData().toString();
    result.type = static_cast<ScheduledActionType>(actionCombo_->currentData().toInt());
    result.value = valueEdit_->text().trimmed();
    result.time = timeEdit_->time();
    for (int index = 0; index < dayChecks_.size(); ++index) {
        if (dayChecks_.at(index)->isChecked()) {
            result.days.insert(static_cast<Qt::DayOfWeek>(index + 1));
        }
    }
    result.enabled = enabledCheck_->isChecked();
    result.lastExecuted = lastExecuted_;
    return result;
}

void ScheduleEditorDialog::validate() {
    QString error;
    const ScheduledAction current = schedule();
    if (current.name.isEmpty()) {
        error = tr("Enter a schedule name.");
    } else if (current.deviceId.isEmpty()) {
        error = tr("Select a device.");
    } else if (current.days.isEmpty()) {
        error = tr("Select at least one weekday.");
    } else if (current.type == ScheduledActionType::SetBrightness) {
        bool ok = false;
        const int value = current.value.toInt(&ok);
        if (!ok || value < 1 || value > 100) {
            error = tr("Brightness must be between 1 and 100.");
        }
    } else if (current.type == ScheduledActionType::ApplyPreset
               && QUuid(current.value.toString()).isNull()) {
        error = tr("Enter a valid custom preset UUID.");
    }
    errorLabel_->setText(error);
    buttons_->button(QDialogButtonBox::Save)->setEnabled(error.isEmpty());
}

