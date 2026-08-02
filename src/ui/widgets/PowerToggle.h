#pragma once

#include <QAbstractButton>
#include <QSize>

class PowerToggle final : public QAbstractButton {
public:
    explicit PowerToggle(QWidget* parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};
