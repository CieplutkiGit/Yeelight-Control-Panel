#include "EffectTimelineWidget.h"

#include <QPainter>

EffectTimelineWidget::EffectTimelineWidget(QWidget* parent)
    : QWidget(parent) {
    setAccessibleName(tr("Effect step timeline"));
}

void EffectTimelineWidget::setEffect(const EffectPreset& effect) {
    effect_ = effect;
    update();
}

QSize EffectTimelineWidget::sizeHint() const {
    return {640, 48};
}

void EffectTimelineWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    if (effect_.steps.isEmpty()) {
        return;
    }
    qint64 totalDuration = 0;
    for (const auto& step : effect_.steps) {
        totalDuration += step.durationMs;
    }
    if (totalDuration <= 0) {
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    qreal left = 0;
    for (const auto& step : effect_.steps) {
        const qreal widthFraction = static_cast<qreal>(step.durationMs)
            / static_cast<qreal>(totalDuration);
        const qreal stepWidth = widthFraction * static_cast<qreal>(width());
        QColor color;
        switch (step.mode) {
        case EffectStep::Mode::Rgb:
            color = QColor(
                (step.value >> 16) & 0xff,
                (step.value >> 8) & 0xff,
                step.value & 0xff
            );
            break;
        case EffectStep::Mode::ColorTemperature:
            color = step.value < 3500
                ? QColor(QStringLiteral("#ffb35c"))
                : QColor(QStringLiteral("#dcecff"));
            break;
        case EffectStep::Mode::Sleep:
            color = palette().mid().color();
            break;
        }
        painter.setPen(palette().dark().color());
        painter.setBrush(color);
        painter.drawRoundedRect(
            QRectF(left + 1, 4, qMax<qreal>(2, stepWidth - 2), height() - 8),
            5,
            5
        );
        left += stepWidth;
    }
}
