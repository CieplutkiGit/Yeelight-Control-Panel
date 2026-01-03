#include "LogsPage.h"

#include "../../core/logging/AppLogger.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLineEdit>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

class LogFilterModel final : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    int severity = -1;
    QString device;
    QString category;
    QString text;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const int rowSeverity = index.data(AppLogger::SeverityRole).toInt();
        const QString rowDevice = index.data(AppLogger::DeviceRole).toString();
        const QString rowCategory = index.data(AppLogger::CategoryRole).toString();
        const QString rowText = index.data(AppLogger::MessageRole).toString();
        return (severity < 0 || rowSeverity == severity)
            && (device.isEmpty() || rowDevice.contains(device, Qt::CaseInsensitive))
            && (category.isEmpty() || rowCategory.contains(category, Qt::CaseInsensitive))
            && (text.isEmpty() || rowText.contains(text, Qt::CaseInsensitive));
    }
};

LogsPage::LogsPage(SettingsRepository* settings, QWidget* parent)
    : QWidget(parent)
    , settings_(settings)
    , filter_(new LogFilterModel(this))
    , table_(new QTableView(this))
    , severityFilter_(new QComboBox(this))
    , deviceFilter_(new QLineEdit(this))
    , categoryFilter_(new QLineEdit(this))
    , textFilter_(new QLineEdit(this))
    , rawGroup_(new QGroupBox(tr("Developer raw command console"), this))
    , methodEdit_(new QLineEdit(rawGroup_))
    , parametersEdit_(new QPlainTextEdit(rawGroup_))
    , transcript_(new QPlainTextEdit(rawGroup_)) {
    severityFilter_->addItem(tr("All severities"), -1);
    severityFilter_->addItem(tr("Debug"), static_cast<int>(AppLogger::Severity::Debug));
    severityFilter_->addItem(tr("Info"), static_cast<int>(AppLogger::Severity::Info));
    severityFilter_->addItem(tr("Warning"), static_cast<int>(AppLogger::Severity::Warning));
    severityFilter_->addItem(tr("Error"), static_cast<int>(AppLogger::Severity::Error));
    deviceFilter_->setPlaceholderText(tr("Device"));
    categoryFilter_->setPlaceholderText(tr("Category"));
    textFilter_->setPlaceholderText(tr("Search text"));
    filter_->setSourceModel(&AppLogger::instance());
    table_->setModel(filter_);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto* filters = new QHBoxLayout;
    filters->addWidget(severityFilter_);
    filters->addWidget(deviceFilter_);
    filters->addWidget(categoryFilter_);
    filters->addWidget(textFilter_, 1);
    auto* copySelected = new QPushButton(tr("Copy selected"), this);
    auto* copyAll = new QPushButton(tr("Copy all"), this);
    auto* clearButton = new QPushButton(tr("Clear"), this);
    auto* exportButton = new QPushButton(tr("Export to text file"), this);
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(copySelected);
    buttons->addWidget(copyAll);
    buttons->addWidget(clearButton);
    buttons->addWidget(exportButton);
    buttons->addStretch();

    auto* warning = new QLabel(
        tr("Unsupported commands may be rejected. Commands are sent only to "
           "the currently selected LAN device."),
        rawGroup_
    );
    warning->setWordWrap(true);
    methodEdit_->setPlaceholderText(tr("Method, for example get_prop"));
    parametersEdit_->setPlaceholderText(tr("JSON array parameters, for example [\"power\"]"));
    parametersEdit_->setMaximumHeight(72);
    transcript_->setReadOnly(true);
    auto* sendButton = new QPushButton(tr("Send to selected device"), rawGroup_);
    auto* rawForm = new QFormLayout;
    rawForm->addRow(tr("Method"), methodEdit_);
    rawForm->addRow(tr("Parameters"), parametersEdit_);
    auto* rawLayout = new QVBoxLayout(rawGroup_);
    rawLayout->addWidget(warning);
    rawLayout->addLayout(rawForm);
    rawLayout->addWidget(sendButton);
    rawLayout->addWidget(transcript_);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(filters);
    layout->addWidget(table_, 1);
    layout->addLayout(buttons);
    layout->addWidget(rawGroup_);

    const auto refilter = [this] {
        auto* model = static_cast<LogFilterModel*>(filter_);
        model->severity = severityFilter_->currentData().toInt();
        model->device = deviceFilter_->text();
        model->category = categoryFilter_->text();
        model->text = textFilter_->text();
        model->invalidate();
    };
    connect(severityFilter_, &QComboBox::currentIndexChanged, this, refilter);
    connect(deviceFilter_, &QLineEdit::textChanged, this, refilter);
    connect(categoryFilter_, &QLineEdit::textChanged, this, refilter);
    connect(textFilter_, &QLineEdit::textChanged, this, refilter);
    connect(copySelected, &QPushButton::clicked, this,
        [this] { copyRows(true); });
    connect(copyAll, &QPushButton::clicked, this,
        [this] { copyRows(false); });
    connect(clearButton, &QPushButton::clicked,
        &AppLogger::instance(), &AppLogger::clear);
    connect(exportButton, &QPushButton::clicked, this, &LogsPage::exportLogs);
    connect(sendButton, &QPushButton::clicked, this, &LogsPage::sendRawCommand);
    setDeveloperMode(
        settings_ != nullptr
            && settings_->value(QStringLiteral("settings/developerMode"), false).toBool()
    );
}

void LogsPage::setDevice(DeviceController* device) {
    if (device_ != nullptr) {
        disconnect(device_, nullptr, this, nullptr);
    }
    device_ = device;
    if (device_ != nullptr) {
        connect(device_, &DeviceController::rawRequest, this,
            [this](int id, const QByteArray& request) {
                transcript_->appendPlainText(
                    tr("Request %1: %2").arg(id).arg(QString::fromUtf8(request).trimmed())
                );
            });
        connect(device_, &DeviceController::rawResponse, this,
            [this](int id, const QJsonArray& response) {
                transcript_->appendPlainText(
                    tr("Response %1: %2").arg(id).arg(
                        QString::fromUtf8(QJsonDocument(response).toJson(
                            QJsonDocument::Compact
                        ))
                    )
                );
            });
        connect(device_, &DeviceController::commandError, this,
            [this](const QString& message) {
                transcript_->appendPlainText(tr("Error: %1").arg(message));
            });
    }
    rawGroup_->setEnabled(device_ != nullptr && developerMode_);
}

void LogsPage::setDeveloperMode(bool enabled) {
    developerMode_ = enabled;
    rawGroup_->setVisible(enabled);
    rawGroup_->setEnabled(enabled && device_ != nullptr);
    if (settings_ != nullptr) {
        settings_->setValue(QStringLiteral("settings/developerMode"), enabled);
        settings_->sync();
    }
}

void LogsPage::sendRawCommand() {
    if (device_ == nullptr || methodEdit_->text().trimmed().isEmpty()) {
        return;
    }
    QJsonParseError error;
    const QJsonDocument parameters = QJsonDocument::fromJson(
        parametersEdit_->toPlainText().trimmed().toUtf8(),
        &error
    );
    if (error.error != QJsonParseError::NoError || !parameters.isArray()) {
        transcript_->appendPlainText(tr("Error: parameters must be a valid JSON array."));
        return;
    }
    device_->sendRaw(methodEdit_->text().trimmed(), parameters.array());
}

void LogsPage::copyRows(bool selectedOnly) {
    QModelIndexList rows = selectedOnly
        ? table_->selectionModel()->selectedRows()
        : QModelIndexList{};
    if (!selectedOnly) {
        for (int row = 0; row < filter_->rowCount(); ++row) {
            rows.append(filter_->index(row, 0));
        }
    }
    QStringList lines;
    for (const auto& index : rows) {
        lines.append(index.data(Qt::DisplayRole).toString());
    }
    QApplication::clipboard()->setText(lines.join(QLatin1Char('\n')));
}

void LogsPage::exportLogs() {
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export logs"),
        QStringLiteral("yeelight-lan.log"),
        tr("Text files (*.txt *.log)")
    );
    if (!fileName.isEmpty()) {
        AppLogger::instance().exportTo(fileName);
    }
}
