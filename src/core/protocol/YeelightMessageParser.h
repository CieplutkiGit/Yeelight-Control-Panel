#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QObject>
#include <QVariantMap>

class YeelightMessageParser final : public QObject {
    Q_OBJECT

public:
    explicit YeelightMessageParser(QObject* parent = nullptr);
    void feed(const QByteArray& bytes);
    void reset();

signals:
    void responseReceived(int requestId, const QJsonArray& result);
    void commandFailed(int requestId, int code, const QString& message);
    void propertiesChanged(const QVariantMap& properties);
    void protocolError(const QString& message);

private:
    QByteArray buffer_;
};

