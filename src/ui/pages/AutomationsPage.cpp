#include "AutomationsPage.h"

#include "../dialogs/ScheduleEditorDialog.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
QString actionName(ScheduledActionType type) {
    switch (type) {
    case ScheduledActionType::PowerOn: return QStringLiteral("Turn on");
    case ScheduledActionType::PowerOff: return QStringLiteral("Turn off");
    case ScheduledActionType::Toggle: return QStringLiteral("Toggle");
    case ScheduledActionType::SetBrightness: return QStringLiteral("Brightness");
    case ScheduledActionType::ApplyPreset: return QStringLiteral("Preset");
    }
    return {};
}
}

AutomationsPage::AutomationsPage(
    AutomationEngine* engine,
    DeviceManager* devices,
    QWidget* parent
)
    : QWidget(parent)
    , engine_(engine)
    , devices_(devices)
    , table_(new QTableWidget(0, 8, this)) {
    table_->setHorizontalHeaderLabels({
        tr("Enabled"), tr("Name"), tr("Device"), tr("Action"),
        tr("Value"), tr("Time"), tr("Days"), tr("Last result")
    });
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    auto* notice = new QLabel(
        tr("Local schedules run only while Yeelight LAN is open."),
        this
    );
    notice->setWordWrap(true);
    auto* addButton = new QPushButton(tr("Add schedule"), this);
    auto* editButton = new QPushButton(tr("Edit"), this);
    auto* duplicateButton = new QPushButton(tr("Duplicate"), this);
    auto* deleteButton = new QPushButton(tr("Delete"), this);
    auto* runButton = new QPushButton(tr("Run now"), this);
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(addButton);
    buttons->addWidget(editButton);
    buttons->addWidget(duplicateButton);
    buttons->addWidget(deleteButton);
    buttons->addWidget(runButton);
    buttons->addStretch();
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(notice);
    layout->addWidget(table_, 1);
    layout->addLayout(buttons);

    const bool available = engine_ != nullptr;
    for (auto* button : {addButton, editButton, duplicateButton, deleteButton, runButton}) {
        button->setEnabled(available);
    }
    if (engine_ != nullptr) {
        connect(engine_, &AutomationEngine::schedulesChanged,
            this, &AutomationsPage::rebuild);
        connect(engine_, &AutomationEngine::actionFinished, this,
            [this](const QUuid&, bool, const QString&) { rebuild(); });
    }
    connect(addButton, &QPushButton::clicked, this, &AutomationsPage::addSchedule);
    connect(editButton, &QPushButton::clicked, this, &AutomationsPage::editSchedule);
    connect(duplicateButton, &QPushButton::clicked,
        this, &AutomationsPage::duplicateSchedule);
    connect(deleteButton, &QPushButton::clicked,
        this, &AutomationsPage::deleteSchedule);
    connect(runButton, &QPushButton::clicked, this, &AutomationsPage::runSelected);
    rebuild();
}

void AutomationsPage::rebuild() {
    table_->setRowCount(0);
    if (engine_ == nullptr) {
        return;
    }
    const auto schedules = engine_->schedules();
    for (const auto& schedule : schedules) {
        const int row = table_->rowCount();
        table_->insertRow(row);
        table_->setItem(row, 0, new QTableWidgetItem(schedule.enabled ? tr("Yes") : tr("No")));
        table_->setItem(row, 1, new QTableWidgetItem(schedule.name));
        table_->setItem(row, 2, new QTableWidgetItem(schedule.deviceId));
        table_->setItem(row, 3, new QTableWidgetItem(actionName(schedule.type)));
        table_->setItem(row, 4, new QTableWidgetItem(schedule.value.toString()));
        table_->setItem(row, 5, new QTableWidgetItem(schedule.time.toString(QStringLiteral("HH:mm"))));
        QStringList days;
        for (Qt::DayOfWeek day : schedule.days) {
            days.append(QString::number(static_cast<int>(day)));
        }
        table_->setItem(row, 6, new QTableWidgetItem(days.join(QLatin1Char(','))));
        table_->setItem(
            row,
            7,
            new QTableWidgetItem(
                schedule.lastExecuted.isValid()
                    ? schedule.lastExecuted.toString(Qt::ISODate)
                    : tr("Never")
            )
        );
        table_->item(row, 0)->setData(Qt::UserRole, schedule.id);
    }
}

void AutomationsPage::addSchedule() {
    ScheduleEditorDialog dialog(devices_, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto schedules = engine_->schedules();
        schedules.append(dialog.schedule());
        engine_->setSchedules(schedules);
    }
}

void AutomationsPage::editSchedule() {
    const int row = selectedRow();
    if (row < 0) {
        return;
    }
    auto schedules = engine_->schedules();
    ScheduleEditorDialog dialog(devices_, this);
    dialog.setSchedule(schedules.at(row));
    if (dialog.exec() == QDialog::Accepted) {
        schedules[row] = dialog.schedule();
        engine_->setSchedules(schedules);
    }
}

void AutomationsPage::duplicateSchedule() {
    const int row = selectedRow();
    if (row < 0) {
        return;
    }
    auto schedules = engine_->schedules();
    ScheduledAction copy = schedules.at(row);
    copy.id = QUuid::createUuid();
    copy.name += tr(" copy");
    copy.lastExecuted = {};
    schedules.append(copy);
    engine_->setSchedules(schedules);
}

void AutomationsPage::deleteSchedule() {
    const int row = selectedRow();
    if (row < 0 || QMessageBox::question(
            this,
            tr("Delete schedule"),
            tr("Delete the selected local schedule?")
        ) != QMessageBox::Yes) {
        return;
    }
    auto schedules = engine_->schedules();
    schedules.removeAt(row);
    engine_->setSchedules(schedules);
}

void AutomationsPage::runSelected() {
    const int row = selectedRow();
    if (row >= 0) {
        engine_->runNow(engine_->schedules().at(row).id);
    }
}

int AutomationsPage::selectedRow() const {
    const int row = table_->currentRow();
    return engine_ != nullptr
            && row >= 0
            && row < static_cast<int>(engine_->schedules().size())
        ? row
        : -1;
}
