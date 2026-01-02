#include "AppLogger.h"

#include <QDebug>
#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>

namespace {
constexpr qsizetype MaximumEntries = 2000;
constexpr qint64 MaximumLogFileBytes = 1024 * 1024;

QString formatEntry(const AppLogger::Entry& entry) {
    return QStringLiteral("%1 [%2] [%3] %4%5")
        .arg(
            entry.timestamp.toString(Qt::ISODateWithMs),
            AppLogger::staticMetaObject.enumerator(
                AppLogger::staticMetaObject.indexOfEnumerator("Severity")
            ).valueToKey(static_cast<int>(entry.severity)),
            entry.category,
            entry.device.isEmpty()
                ? QString()
                : QStringLiteral("[%1] ").arg(entry.device),
            entry.message
        );
}
}

AppLogger& AppLogger::instance() {
    static AppLogger logger;
    return logger;
}

AppLogger::AppLogger(QObject* parent)
    : QAbstractListModel(parent) {
}

int AppLogger::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(entries_.size());
}

QVariant AppLogger::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(index.row());
    switch (role) {
    case TimestampRole: return entry.timestamp;
    case SeverityRole: return static_cast<int>(entry.severity);
    case DeviceRole: return entry.device;
    case CategoryRole: return entry.category;
    case MessageRole:
    case Qt::DisplayRole: return formatEntry(entry);
    default: return {};
    }
}

QHash<int, QByteArray> AppLogger::roleNames() const {
    return {
        {TimestampRole, "timestamp"},
        {SeverityRole, "severity"},
        {DeviceRole, "device"},
        {CategoryRole, "category"},
        {MessageRole, "message"}
    };
}

void AppLogger::log(
    Severity severity,
    const QString& category,
    const QString& message,
    const QString& device
) {
    const Entry entry{
        QDateTime::currentDateTimeUtc(),
        severity,
        device,
        category,
        message
    };
    if (entries_.size() >= MaximumEntries) {
        beginRemoveRows({}, 0, 0);
        entries_.removeFirst();
        endRemoveRows();
    }
    const int row = static_cast<int>(entries_.size());
    beginInsertRows({}, row, row);
    entries_.append(entry);
    endInsertRows();

    const QString formatted = formatEntry(entry);
    switch (severity) {
    case Severity::Debug: qDebug().noquote() << formatted; break;
    case Severity::Info: qInfo().noquote() << formatted; break;
    case Severity::Warning: qWarning().noquote() << formatted; break;
    case Severity::Error: qCritical().noquote() << formatted; break;
    }
    appendToFile(entry);
}

void AppLogger::clear() {
    if (entries_.isEmpty()) {
        return;
    }
    beginResetModel();
    entries_.clear();
    endResetModel();
}

QList<AppLogger::Entry> AppLogger::entries() const {
    return entries_;
}

bool AppLogger::exportTo(const QString& fileName) const {
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream stream(&file);
    for (const auto& entry : entries_) {
        stream << formatEntry(entry) << '\n';
    }
    return file.commit();
}

bool AppLogger::setFileLoggingEnabled(bool enabled, const QString& fileName) {
    logFile_.close();
    if (!enabled) {
        logFile_.setFileName({});
        return true;
    }
    if (fileName.isEmpty()) {
        return false;
    }
    logFile_.setFileName(fileName);
    return logFile_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

void AppLogger::appendToFile(const Entry& entry) {
    if (!logFile_.isOpen()) {
        return;
    }
    if (logFile_.size() >= MaximumLogFileBytes) {
        const QString oldName = logFile_.fileName() + QStringLiteral(".1");
        logFile_.close();
        QFile::remove(oldName);
        QFile::rename(logFile_.fileName(), oldName);
        logFile_.open(QIODevice::WriteOnly | QIODevice::Text);
    }
    QTextStream stream(&logFile_);
    stream << formatEntry(entry) << '\n';
    stream.flush();
}

QString AppLogger::severityName(Severity severity) {
    switch (severity) {
    case Severity::Debug: return QStringLiteral("Debug");
    case Severity::Info: return QStringLiteral("Info");
    case Severity::Warning: return QStringLiteral("Warning");
    case Severity::Error: return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}

