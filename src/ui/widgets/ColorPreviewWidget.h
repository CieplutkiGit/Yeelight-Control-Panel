#pragma once

#include <QColor>
#include <QWidget>

class ColorPreviewWidget final : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit ColorPreviewWidget(QWidget* parent = nullptr);

    [[nodiscard]] QColor color() const;
    void setColor(const QColor& color);
    QSize sizeHint() const override;

signals:
    void clicked();
    void colorChanged(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QColor color_ = Qt::white;
};

