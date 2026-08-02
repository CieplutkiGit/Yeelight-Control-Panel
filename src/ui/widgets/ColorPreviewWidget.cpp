#include "ColorPreviewWidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QConicalGradient>
#include <QPainter>
#include <QtMath>

ColorPreviewWidget::ColorPreviewWidget(QWidget* parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Selected color"));
}

QColor ColorPreviewWidget::color() const {
    return color_;
}

void ColorPreviewWidget::setColor(const QColor& color) {
    if (!color.isValid() || color_ == color) {
        return;
    }
    color_ = color;
    update();
    emit colorChanged(color_);
}

QSize ColorPreviewWidget::sizeHint() const {
    return {96, 96};
}

void ColorPreviewWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QPointF center = rect().center();
    const qreal radius = qMin(width(), height()) / 2.0 - 6.0;
    QConicalGradient wheel(center, -90.0);
    wheel.setColorAt(0.00, QColor(QStringLiteral("#ff3030")));
    wheel.setColorAt(0.16, QColor(QStringLiteral("#ffff25")));
    wheel.setColorAt(0.33, QColor(QStringLiteral("#39ef59")));
    wheel.setColorAt(0.50, QColor(QStringLiteral("#2de8f4")));
    wheel.setColorAt(0.67, QColor(QStringLiteral("#3e58ff")));
    wheel.setColorAt(0.83, QColor(QStringLiteral("#cb45ff")));
    wheel.setColorAt(1.00, QColor(QStringLiteral("#ff3030")));
    painter.setPen(QPen(
        hasFocus() ? palette().highlight().color() : QColor(QStringLiteral("#dfe7f3")),
        hasFocus() ? 3 : 1
    ));
    painter.setBrush(wheel);
    painter.drawEllipse(center, radius, radius);

    float hue = 0.0F;
    float saturation = 0.0F;
    float value = 1.0F;
    color_.getHsvF(&hue, &saturation, &value);
    if (hue < 0.0F) {
        hue = 0.08F;
    }
    const qreal angle = (hue * 360.0 - 90.0) * 3.141592653589793 / 180.0;
    const QPointF marker = center + QPointF(
        qCos(angle) * radius * saturation,
        qSin(angle) * radius * saturation
    );
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::white, 4));
    painter.drawEllipse(marker, 11, 11);
}

void ColorPreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint())) {
        emit clicked();
    }
    QWidget::mouseReleaseEvent(event);
}

void ColorPreviewWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
        emit clicked();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

