#include "PowerToggle.h"

#include <QPainter>
#include <QPaintEvent>

PowerToggle::PowerToggle(QWidget* parent)
    : QAbstractButton(parent) {
    setObjectName(QStringLiteral("powerToggle"));
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Power"));
}

QSize PowerToggle::sizeHint() const {
    return {66, 34};
}

void PowerToggle::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF track = QRectF(rect()).adjusted(1, 4, -1, -4);
    const QColor trackColor = isChecked()
        ? QColor(QStringLiteral("#9b66f4"))
        : QColor(QStringLiteral("#2b3544"));
    painter.setPen(QPen(
        hasFocus() ? QColor(QStringLiteral("#c8a9ff")) : trackColor,
        hasFocus() ? 2 : 1
    ));
    painter.setBrush(trackColor);
    painter.drawRoundedRect(track, track.height() / 2, track.height() / 2);

    const qreal diameter = track.height() - 6;
    const qreal left = isChecked()
        ? track.right() - diameter - 3
        : track.left() + 3;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#f7f4ff")));
    painter.drawEllipse(QRectF(left, track.top() + 3, diameter, diameter));
}
