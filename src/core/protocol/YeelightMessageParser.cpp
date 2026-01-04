#include "YeelightMessageParser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(protocolLog, "protocol")

YeelightMessageParser::YeelightMessageParser(QObject* parent)
    : QObject(parent) {
}

void YeelightMessageParser::feed(const QByteArray& bytes) {
    buffer_.append(bytes);
    qsizetype separator = -1;
    while ((separator = buffer_.indexOf("\r\n")) >= 0) {
        const QByteArray line = buffer_.left(separator);
        buffer_.remove(0, separator + 2);
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            const QString message = QStringLiteral("Malformed Yeelight message: %1")
                .arg(parseError.errorString());
            qCWarning(protocolLog, "%s", qUtf8Printable(message));
            emit protocolError(message);
            continue;
        }

        const QJsonObject object = document.object();
        if (object.contains(QStringLiteral("id"))) {
            const int id = object.value(QStringLiteral("id")).toInt(-1);
            if (id < 0) {
                emit protocolError(QStringLiteral("Response has an invalid request ID."));
            } else if (object.value(QStringLiteral("result")).isArray()) {
                emit responseReceived(id, object.value(QStringLiteral("result")).toArray());
            } else if (object.value(QStringLiteral("error")).isObject()) {
                const QJsonObject error = object.value(QStringLiteral("error")).toObject();
                emit commandFailed(
                    id,
                    error.value(QStringLiteral("code")).toInt(),
                    error.value(QStringLiteral("message")).toString()
                );
            } else {
                emit protocolError(QStringLiteral("Response contains neither a result nor an error."));
            }
            continue;
        }

        if (object.value(QStringLiteral("method")).toString() == QStringLiteral("props")
            && object.value(QStringLiteral("params")).isObject()) {
            emit propertiesChanged(
                object.value(QStringLiteral("params")).toObject().toVariantMap()
            );
            continue;
        }

        emit protocolError(QStringLiteral("Unknown Yeelight notification."));
    }
}

void YeelightMessageParser::reset() {
    buffer_.clear();
}

