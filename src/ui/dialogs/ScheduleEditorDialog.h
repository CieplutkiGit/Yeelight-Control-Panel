#pragma once

#include "../../core/device/DeviceManager.h"
#include "../../core/model/ScheduledAction.h"

#include <QDialog>
#include <QList>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QTimeEdit;

class ScheduleEditorDialog final : public QDialog {
    Q_OBJECT

public:
    explicit ScheduleEditorDialog(
        DeviceManager* devices,
        QWidget* parent = nullptr
    );

    void setSchedule(const ScheduledAction& schedule);
    [[nodiscard]] ScheduledAction schedule() const;

private:
    void validate();

    QUuid id_;
    QLineEdit* nameEdit_;
    QComboBox* deviceCombo_;
    QComboBox* actionCombo_;
    QLineEdit* valueEdit_;
    QTimeEdit* timeEdit_;
    QList<QCheckBox*> dayChecks_;
    QCheckBox* enabledCheck_;
    QLabel* errorLabel_;
    QDialogButtonBox* buttons_;
    QDateTime lastExecuted_;
};

