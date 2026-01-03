#pragma once

#include "../../core/model/EffectPreset.h"

#include <QDialog>

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTableWidget;
class EffectTimelineWidget;

class EffectEditorDialog final : public QDialog {
    Q_OBJECT

public:
    explicit EffectEditorDialog(bool developerMode, QWidget* parent = nullptr);

    void setEffect(const EffectPreset& effect);
    [[nodiscard]] EffectPreset effect() const;

private:
    void addStep(const EffectStep& step = {});
    void duplicateStep();
    void removeStep();
    void moveStep(int offset);
    void validate();
    void updateExpression();

    bool developerMode_;
    QUuid effectId_;
    QLineEdit* nameEdit_;
    QSpinBox* repeatSpin_;
    QComboBox* finishCombo_;
    QTableWidget* stepTable_;
    EffectTimelineWidget* timeline_;
    QLabel* errorLabel_;
    QLabel* expressionLabel_;
    QDialogButtonBox* buttons_;
};
