#pragma once

#include "../../core/model/EffectPreset.h"

#include <QWidget>

class EffectTimelineWidget final : public QWidget {
    Q_OBJECT

public:
    explicit EffectTimelineWidget(QWidget* parent = nullptr);
    void setEffect(const EffectPreset& effect);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    EffectPreset effect_;
};

