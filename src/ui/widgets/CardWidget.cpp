#include "CardWidget.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

CardWidget::CardWidget(const QString& title, QWidget* parent)
    : CardWidget(title, {}, parent) {}

CardWidget::CardWidget(const QString& title, const QString& iconPath, QWidget* parent)
    : QFrame(parent)
    , titleLabel_(new QLabel(title, this))
    , iconLabel_(new QLabel(this))
    , contentLayout_(new QVBoxLayout) {
    setObjectName(QStringLiteral("card"));
    setFrameShape(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    titleLabel_->setObjectName(QStringLiteral("cardTitle"));
    titleLabel_->setVisible(!title.isEmpty());
    iconLabel_->setObjectName(QStringLiteral("cardIcon"));
    iconLabel_->setFixedSize(24, 24);
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setVisible(!iconPath.isEmpty());
    if (!iconPath.isEmpty()) {
        iconLabel_->setPixmap(QIcon(iconPath).pixmap(22, 22));
    }
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 18);
    layout->setSpacing(12);
    auto* titleRow = new QHBoxLayout;
    titleRow->setSpacing(10);
    titleRow->addWidget(iconLabel_);
    titleRow->addWidget(titleLabel_);
    titleRow->addStretch();
    layout->addLayout(titleRow);
    layout->addLayout(contentLayout_);
}

QVBoxLayout* CardWidget::contentLayout() const {
    return contentLayout_;
}
