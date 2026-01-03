#include "SegmentPreviewWidget.h"

#include <QPainter>

SegmentPreviewWidget::SegmentPreviewWidget(QWidget* parent)
    : QWidget(parent)
    , colors_({Qt::red, Qt::yellow, Qt::green, Qt::cyan, Qt::blue, Qt::magenta}) {
    setAccessibleName(tr("Segment color preview"));
}

void SegmentPreviewWidget::setSegments(const QList<QColor>& colors) {
    colors_ = colors;
    update();
}

QSize SegmentPreviewWidget::sizeHint() const {
    return {480, 48};
}

void SegmentPreviewWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    if (colors_.isEmpty()) {
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal segmentWidth = static_cast<qreal>(width())
        / static_cast<qreal>(colors_.size());
    for (qsizetype index = 0; index < colors_.size(); ++index) {
        QRectF segment(
            static_cast<qreal>(index) * segmentWidth,
            4,
            segmentWidth,
            height() - 8
        );
        painter.setPen(palette().mid().color());
        painter.setBrush(colors_.at(index));
        painter.drawRoundedRect(segment.adjusted(2, 0, -2, 0), 6, 6);
    }
}

