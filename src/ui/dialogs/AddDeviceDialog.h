#pragma once

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QSpinBox;

class AddDeviceDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AddDeviceDialog(QWidget* parent = nullptr);

    [[nodiscard]] QString address() const;
    [[nodiscard]] quint16 port() const;
    [[nodiscard]] QString displayName() const;
    [[nodiscard]] bool rememberDevice() const;
    [[nodiscard]] bool connectImmediately() const;

private:
    void validate();

    QLineEdit* addressEdit_;
    QSpinBox* portSpin_;
    QLineEdit* nameEdit_;
    QCheckBox* rememberCheck_;
    QCheckBox* connectCheck_;
    QLabel* errorLabel_;
    QDialogButtonBox* buttons_;
};

