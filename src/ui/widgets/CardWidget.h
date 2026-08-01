#pragma once

#include <QFrame>

class QLabel;
class QVBoxLayout;

class CardWidget final : public QFrame {
    Q_OBJECT

public:
    explicit CardWidget(
        const QString& title = {},
        QWidget* parent = nullptr
    );

    [[nodiscard]] QVBoxLayout* contentLayout() const;

private:
    QLabel* titleLabel_;
    QVBoxLayout* contentLayout_;
};
