#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QFile>
#include <QList>

class AppLogger final : public QAbstractListModel {
    Q_OBJECT

public:
    enum class Severity {
        Debug,
        Info,
        Warning,
        Error
    };
    Q_ENUM(Severity)

    struct Entry {
        QDateTime timestamp;
        Severity severity = Severity::Info;
        QString device;
        QString category;
        QString message;
    };

    enum Role {
        TimestampRole = Qt::UserRole + 1,
        SeverityRole,
        DeviceRole,
        CategoryRole,
        MessageRole
    };

    static AppLogger& instance();

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void log(
        Severity severity,
        const QString& category,
        const QString& message,
        const QString& device = {}
    );
    void clear();
    [[nodiscard]] QList<Entry> entries() const;
    bool exportTo(const QString& fileName) const;
    bool setFileLoggingEnabled(bool enabled, const QString& fileName = {});

private:
    explicit AppLogger(QObject* parent = nullptr);
    void appendToFile(const Entry& entry);
    QList<Entry> entries_;
    QFile logFile_;
};
