#pragma once

#include <QColor>
#include <QList>
#include <QWidget>

class SegmentPreviewWidget final : public QWidget {
    Q_OBJECT

public:
    explicit SegmentPreviewWidget(QWidget* parent = nullptr);
    void setSegments(const QList<QColor>& colors);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<QColor> colors_;
};

