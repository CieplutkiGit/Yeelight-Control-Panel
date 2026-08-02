#pragma once

#include <QFrame>

class QLabel;
class QHBoxLayout;
class QVBoxLayout;

class CardWidget final : public QFrame {
    Q_OBJECT

public:
    explicit CardWidget(
        const QString& title = {},
        QWidget* parent = nullptr
    );
    CardWidget(
        const QString& title,
        const QString& iconPath,
        QWidget* parent = nullptr
    );

    [[nodiscard]] QVBoxLayout* contentLayout() const;

private:
    QLabel* titleLabel_;
    QLabel* iconLabel_;
    QVBoxLayout* contentLayout_;
};
