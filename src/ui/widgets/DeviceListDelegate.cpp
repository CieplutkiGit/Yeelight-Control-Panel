#include "DeviceListDelegate.h"

#include "../models/DeviceListModel.h"

#include <QPainter>

void DeviceListDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    painter->save();
    const bool selected = option.state.testFlag(QStyle::State_Selected);
    if (selected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.highlight());
        painter->drawRoundedRect(option.rect.adjusted(2, 2, -2, -2), 8, 8);
    }

    const bool online = index.data(DeviceListModel::OnlineRole).toBool();
    painter->setPen(Qt::NoPen);
    painter->setBrush(online ? QColor(QStringLiteral("#35b36f"))
                             : option.palette.mid().color());
    painter->drawEllipse(
        QPoint(option.rect.left() + 14, option.rect.center().y()),
        5,
        5
    );

    const QStringList lines = index.data(Qt::DisplayRole).toString().split(QLatin1Char('\n'));
    const QColor textColor = selected
        ? option.palette.highlightedText().color()
        : option.palette.text().color();
    painter->setPen(textColor);
    QFont titleFont = option.font;
    titleFont.setWeight(QFont::DemiBold);
    painter->setFont(titleFont);
    painter->drawText(
        QRect(option.rect.left() + 28, option.rect.top() + 9,
              option.rect.width() - 36, 22),
        Qt::AlignLeft | Qt::AlignVCenter,
        lines.value(0)
    );
    painter->setFont(option.font);
    painter->setPen(selected ? textColor : option.palette.placeholderText().color());
    painter->drawText(
        QRect(option.rect.left() + 28, option.rect.top() + 33,
              option.rect.width() - 80, 20),
        Qt::AlignLeft | Qt::AlignVCenter,
        lines.value(1)
    );
    const int power = index.data(DeviceListModel::PowerRole).toInt();
    const QString powerText = power == static_cast<int>(PowerState::On)
        ? tr("On")
        : (power == static_cast<int>(PowerState::Off) ? tr("Off") : tr("—"));
    painter->drawText(
        QRect(option.rect.right() - 48, option.rect.top() + 33, 40, 20),
        Qt::AlignRight | Qt::AlignVCenter,
        powerText
    );
    painter->restore();
}

QSize DeviceListDelegate::sizeHint(
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    Q_UNUSED(option)
    Q_UNUSED(index)
    return {240, 64};
}

