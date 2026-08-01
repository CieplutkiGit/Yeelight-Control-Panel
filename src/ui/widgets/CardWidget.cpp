#include "CardWidget.h"

#include <QLabel>
#include <QVBoxLayout>

CardWidget::CardWidget(const QString& title, QWidget* parent)
    : QFrame(parent)
    , titleLabel_(new QLabel(title, this))
    , contentLayout_(new QVBoxLayout) {
    setObjectName(QStringLiteral("card"));
    setFrameShape(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    titleLabel_->setObjectName(QStringLiteral("cardTitle"));
    titleLabel_->setVisible(!title.isEmpty());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 18);
    layout->setSpacing(12);
    layout->addWidget(titleLabel_);
    layout->addLayout(contentLayout_);
}

QVBoxLayout* CardWidget::contentLayout() const {
    return contentLayout_;
}
