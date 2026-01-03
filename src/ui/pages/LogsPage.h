#pragma once

#include "../../core/device/DeviceController.h"
#include "../../core/persistence/SettingsRepository.h"

#include <QWidget>

class QComboBox;
class QGroupBox;
class QLineEdit;
class QPlainTextEdit;
class QSortFilterProxyModel;
class QTableView;

class LogsPage final : public QWidget {
    Q_OBJECT

public:
    explicit LogsPage(
        SettingsRepository* settings,
        QWidget* parent = nullptr
    );
    void setDevice(DeviceController* device);
    void setDeveloperMode(bool enabled);

private:
    void sendRawCommand();
    void copyRows(bool selectedOnly);
    void exportLogs();

    SettingsRepository* settings_;
    DeviceController* device_ = nullptr;
    QSortFilterProxyModel* filter_;
    QTableView* table_;
    QComboBox* severityFilter_;
    QLineEdit* deviceFilter_;
    QLineEdit* categoryFilter_;
    QLineEdit* textFilter_;
    QGroupBox* rawGroup_;
    QLineEdit* methodEdit_;
    QPlainTextEdit* parametersEdit_;
    QPlainTextEdit* transcript_;
    bool developerMode_ = false;
};
