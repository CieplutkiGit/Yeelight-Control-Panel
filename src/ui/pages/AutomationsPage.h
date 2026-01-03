#pragma once

#include "../../core/automation/AutomationEngine.h"

#include <QWidget>

class QTableWidget;

class AutomationsPage final : public QWidget {
    Q_OBJECT

public:
    explicit AutomationsPage(
        AutomationEngine* engine,
        DeviceManager* devices,
        QWidget* parent = nullptr
    );

private:
    void rebuild();
    void addSchedule();
    void editSchedule();
    void duplicateSchedule();
    void deleteSchedule();
    void runSelected();
    [[nodiscard]] int selectedRow() const;

    AutomationEngine* engine_;
    DeviceManager* devices_;
    QTableWidget* table_;
};

