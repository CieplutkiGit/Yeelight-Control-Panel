#pragma once

#include <QLabel>

class ConnectionBadge final : public QLabel {
    Q_OBJECT

public:
    explicit ConnectionBadge(QWidget* parent = nullptr);
    void setOnline(bool online);
};

