#include "DeviceListDelegate.h"

#include "../models/DeviceListModel.h"

#include <QIcon>
#include <QPainter>

void DeviceListDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    painter->save();
    const bool selected = option.state.testFlag(QStyle::State_Selected);
    if (selected) {
        painter->setPen(QPen(QColor(QStringLiteral("#a26cf5")), 1));
        painter->setBrush(QColor(QStringLiteral("#2b2449")));
        painter->drawRoundedRect(option.rect.adjusted(2, 2, -2, -2), 10, 10);
    } else {
        painter->setPen(QPen(QColor(QStringLiteral("#2b3544")), 1));
        painter->setBrush(QColor(QStringLiteral("#18212c")));
        painter->drawRoundedRect(option.rect.adjusted(2, 2, -2, -2), 10, 10);
    }

    const bool online = index.data(DeviceListModel::OnlineRole).toBool();
    QIcon(QStringLiteral(":/icons/bulb.svg")).paint(
        painter,
        QRect(option.rect.left() + 12, option.rect.top() + 14, 58, 72),
        Qt::AlignCenter
    );

    const QStringList lines = index.data(Qt::DisplayRole).toString().split(QLatin1Char('\n'));
    const QColor textColor = selected
        ? QColor(QStringLiteral("#f2efff"))
        : QColor(QStringLiteral("#edf1f8"));
    painter->setPen(textColor);
    QFont titleFont = option.font;
    titleFont.setWeight(QFont::DemiBold);
    painter->setFont(titleFont);
    painter->drawText(
        QRect(option.rect.left() + 84, option.rect.top() + 16,
              option.rect.width() - 112, 24),
        Qt::AlignLeft | Qt::AlignVCenter,
        lines.value(0).remove(QChar(0x2605)).trimmed()
    );

    painter->setFont(option.font);
    painter->setPen(QColor(QStringLiteral("#a1adbf")));
    painter->drawText(
        QRect(option.rect.left() + 84, option.rect.top() + 44,
              option.rect.width() - 96, 20),
        Qt::AlignLeft | Qt::AlignVCenter,
        lines.value(1)
    );

    painter->setPen(online ? QColor(QStringLiteral("#8fe776"))
                           : QColor(QStringLiteral("#7c8798")));
    painter->drawEllipse(QPoint(option.rect.left() + 88, option.rect.top() + 78), 4, 4);
    painter->drawText(
        QRect(option.rect.left() + 98, option.rect.top() + 68,
              option.rect.width() - 118, 20),
        Qt::AlignLeft | Qt::AlignVCenter,
        online ? tr("Online") : tr("Offline")
    );

    if (index.data(DeviceListModel::FavoriteRole).toBool()) {
        painter->setPen(QColor(QStringLiteral("#d7dbe4")));
        painter->drawText(
            QRect(option.rect.right() - 32, option.rect.top() + 12, 20, 22),
            Qt::AlignCenter,
            QString(QChar(0x2606))
        );
    }
    painter->restore();
}

QSize DeviceListDelegate::sizeHint(
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    Q_UNUSED(option)
    Q_UNUSED(index)
    return {280, 110};
}
