#include "EffectEditorDialog.h"

#include "../../core/protocol/YeelightCommand.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
QString modeName(EffectStep::Mode mode) {
    switch (mode) {
    case EffectStep::Mode::Rgb: return QStringLiteral("RGB");
    case EffectStep::Mode::ColorTemperature: return QStringLiteral("Temperature");
    case EffectStep::Mode::Sleep: return QStringLiteral("Sleep");
    }
    return QStringLiteral("RGB");
}

EffectStep::Mode modeValue(const QString& name) {
    if (name == QStringLiteral("Temperature")) {
        return EffectStep::Mode::ColorTemperature;
    }
    if (name == QStringLiteral("Sleep")) {
        return EffectStep::Mode::Sleep;
    }
    return EffectStep::Mode::Rgb;
}
}

EffectEditorDialog::EffectEditorDialog(bool developerMode, QWidget* parent)
    : QDialog(parent)
    , developerMode_(developerMode)
    , effectId_(QUuid::createUuid())
    , nameEdit_(new QLineEdit(this))
    , repeatSpin_(new QSpinBox(this))
    , finishCombo_(new QComboBox(this))
    , stepTable_(new QTableWidget(0, 4, this))
    , errorLabel_(new QLabel(this))
    , expressionLabel_(new QLabel(this))
    , buttons_(new QDialogButtonBox(
          QDialogButtonBox::Save | QDialogButtonBox::Cancel,
          this
      )) {
    setWindowTitle(tr("Effect editor"));
    resize(760, 520);
    nameEdit_->setPlaceholderText(tr("Effect name"));
    repeatSpin_->setRange(0, 1000);
    repeatSpin_->setSpecialValueText(tr("Infinite"));
    finishCombo_->addItems({tr("Recover"), tr("Stay"), tr("Turn off")});
    stepTable_->setHorizontalHeaderLabels({
        tr("Duration (ms)"),
        tr("Mode"),
        tr("Value"),
        tr("Brightness")
    });
    stepTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stepTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    stepTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    errorLabel_->setObjectName(QStringLiteral("validationErrorLabel"));
    errorLabel_->setWordWrap(true);
    expressionLabel_->setWordWrap(true);
    expressionLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    expressionLabel_->setVisible(developerMode_);

    auto* form = new QFormLayout;
    form->addRow(tr("Name"), nameEdit_);
    form->addRow(tr("Repeat count"), repeatSpin_);
    form->addRow(tr("Finish action"), finishCombo_);
    auto* addButton = new QPushButton(tr("Add step"), this);
    auto* duplicateButton = new QPushButton(tr("Duplicate step"), this);
    auto* removeButton = new QPushButton(tr("Remove step"), this);
    auto* upButton = new QPushButton(tr("Move up"), this);
    auto* downButton = new QPushButton(tr("Move down"), this);
    auto* rowButtons = new QHBoxLayout;
    rowButtons->addWidget(addButton);
    rowButtons->addWidget(duplicateButton);
    rowButtons->addWidget(removeButton);
    rowButtons->addWidget(upButton);
    rowButtons->addWidget(downButton);
    rowButtons->addStretch();
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(stepTable_, 1);
    layout->addLayout(rowButtons);
    layout->addWidget(errorLabel_);
    layout->addWidget(expressionLabel_);
    layout->addWidget(buttons_);

    connect(addButton, &QPushButton::clicked, this, [this] { addStep(); });
    connect(duplicateButton, &QPushButton::clicked,
        this, &EffectEditorDialog::duplicateStep);
    connect(removeButton, &QPushButton::clicked,
        this, &EffectEditorDialog::removeStep);
    connect(upButton, &QPushButton::clicked, this,
        [this] { moveStep(-1); });
    connect(downButton, &QPushButton::clicked, this,
        [this] { moveStep(1); });
    connect(nameEdit_, &QLineEdit::textChanged, this, &EffectEditorDialog::validate);
    connect(repeatSpin_, &QSpinBox::valueChanged, this, &EffectEditorDialog::validate);
    connect(finishCombo_, &QComboBox::currentIndexChanged,
        this, &EffectEditorDialog::validate);
    connect(stepTable_, &QTableWidget::cellChanged,
        this, &EffectEditorDialog::validate);
    connect(buttons_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    addStep({500, EffectStep::Mode::Rgb, 16711680, 100});
}

void EffectEditorDialog::setEffect(const EffectPreset& effect) {
    effectId_ = effect.id.isNull() ? QUuid::createUuid() : effect.id;
    nameEdit_->setText(effect.name);
    repeatSpin_->setValue(effect.repeatCount);
    finishCombo_->setCurrentIndex(qBound(0, effect.finishAction, 2));
    stepTable_->setRowCount(0);
    for (const auto& step : effect.steps) {
        addStep(step);
    }
    validate();
}

EffectPreset EffectEditorDialog::effect() const {
    EffectPreset value;
    value.id = effectId_;
    value.name = nameEdit_->text().trimmed();
    value.repeatCount = repeatSpin_->value();
    value.finishAction = finishCombo_->currentIndex();
    for (int row = 0; row < stepTable_->rowCount(); ++row) {
        EffectStep step;
        step.durationMs = stepTable_->item(row, 0)->text().toInt();
        step.mode = modeValue(stepTable_->item(row, 1)->text());
        step.value = stepTable_->item(row, 2)->text().toInt();
        step.brightness = stepTable_->item(row, 3)->text().toInt();
        value.steps.append(step);
    }
    return value;
}

void EffectEditorDialog::addStep(const EffectStep& step) {
    const QSignalBlocker blocker(stepTable_);
    const int row = stepTable_->rowCount();
    stepTable_->insertRow(row);
    stepTable_->setItem(row, 0, new QTableWidgetItem(QString::number(step.durationMs)));
    auto* modeItem = new QTableWidgetItem(modeName(step.mode));
    modeItem->setToolTip(tr("Use RGB, Temperature, or Sleep."));
    stepTable_->setItem(row, 1, modeItem);
    stepTable_->setItem(row, 2, new QTableWidgetItem(QString::number(step.value)));
    stepTable_->setItem(row, 3, new QTableWidgetItem(QString::number(step.brightness)));
    stepTable_->selectRow(row);
    validate();
}

void EffectEditorDialog::duplicateStep() {
    const int row = stepTable_->currentRow();
    if (row < 0) {
        return;
    }
    EffectStep step;
    step.durationMs = stepTable_->item(row, 0)->text().toInt();
    step.mode = modeValue(stepTable_->item(row, 1)->text());
    step.value = stepTable_->item(row, 2)->text().toInt();
    step.brightness = stepTable_->item(row, 3)->text().toInt();
    addStep(step);
}

void EffectEditorDialog::removeStep() {
    const int row = stepTable_->currentRow();
    if (row >= 0) {
        stepTable_->removeRow(row);
        validate();
    }
}

void EffectEditorDialog::moveStep(int offset) {
    const int source = stepTable_->currentRow();
    const int target = source + offset;
    if (source < 0 || target < 0 || target >= stepTable_->rowCount()) {
        return;
    }
    for (int column = 0; column < stepTable_->columnCount(); ++column) {
        const QString sourceText = stepTable_->item(source, column)->text();
        const QString targetText = stepTable_->item(target, column)->text();
        stepTable_->item(source, column)->setText(targetText);
        stepTable_->item(target, column)->setText(sourceText);
    }
    stepTable_->selectRow(target);
}

void EffectEditorDialog::validate() {
    QString error;
    for (int row = 0; row < stepTable_->rowCount() && error.isEmpty(); ++row) {
        bool durationOk = false;
        bool valueOk = false;
        bool brightnessOk = false;
        stepTable_->item(row, 0)->text().toInt(&durationOk);
        stepTable_->item(row, 2)->text().toInt(&valueOk);
        stepTable_->item(row, 3)->text().toInt(&brightnessOk);
        const QString mode = stepTable_->item(row, 1)->text();
        const bool modeOk = mode == QStringLiteral("RGB")
            || mode == QStringLiteral("Temperature")
            || mode == QStringLiteral("Sleep");
        if (!durationOk || !valueOk || !brightnessOk || !modeOk) {
            error = tr("Each step must use numeric values and a valid mode.");
        }
    }
    const EffectPreset current = effect();
    if (error.isEmpty() && current.name.isEmpty()) {
        error = tr("Enter an effect name.");
    } else if (error.isEmpty()) {
        error = current.validationError();
    }
    errorLabel_->setText(error);
    buttons_->button(QDialogButtonBox::Save)->setEnabled(error.isEmpty());
    updateExpression();
}

void EffectEditorDialog::updateExpression() {
    if (!developerMode_) {
        return;
    }
    const auto result = YeelightCommand::startColorFlow(1, effect());
    expressionLabel_->setText(
        result.success
            ? tr("Flow: %1").arg(
                result.command.value(QStringLiteral("params")).toArray()
                    .at(2).toString()
            )
            : tr("Flow unavailable until the effect is valid.")
    );
}
