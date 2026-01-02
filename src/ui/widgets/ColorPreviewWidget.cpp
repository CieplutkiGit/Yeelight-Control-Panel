#include "ColorPreviewWidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

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
    painter.setPen(QPen(
        hasFocus() ? palette().highlight().color() : palette().mid().color(),
        hasFocus() ? 3 : 1
    ));
    painter.setBrush(color_);
    painter.drawRoundedRect(rect().adjusted(3, 3, -3, -3), 10, 10);
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

