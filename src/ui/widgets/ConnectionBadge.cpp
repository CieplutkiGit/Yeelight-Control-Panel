#include "ConnectionBadge.h"

#include <QStyle>

ConnectionBadge::ConnectionBadge(QWidget* parent)
    : QLabel(parent) {
    setObjectName(QStringLiteral("connectionBadge"));
    setAlignment(Qt::AlignCenter);
    setOnline(false);
}

void ConnectionBadge::setOnline(bool online) {
    setText(online ? tr("Online") : tr("Offline"));
    setProperty("online", online);
    style()->unpolish(this);
    style()->polish(this);
}
