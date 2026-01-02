#include "AddDeviceDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

AddDeviceDialog::AddDeviceDialog(QWidget* parent)
    : QDialog(parent)
    , addressEdit_(new QLineEdit(this))
    , portSpin_(new QSpinBox(this))
    , nameEdit_(new QLineEdit(this))
    , rememberCheck_(new QCheckBox(tr("Remember this device"), this))
    , connectCheck_(new QCheckBox(tr("Connect immediately"), this))
    , errorLabel_(new QLabel(this))
    , buttons_(new QDialogButtonBox(
          QDialogButtonBox::Save | QDialogButtonBox::Cancel,
          this
      )) {
    setWindowTitle(tr("Add Yeelight by IP"));
    setModal(true);
    setMinimumWidth(420);
    addressEdit_->setPlaceholderText(tr("192.168.1.25 or IPv6 address"));
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(55443);
    nameEdit_->setPlaceholderText(tr("Optional local display name"));
    rememberCheck_->setChecked(true);
    connectCheck_->setChecked(true);
    errorLabel_->setObjectName(QStringLiteral("validationErrorLabel"));
    errorLabel_->setWordWrap(true);

    auto* form = new QFormLayout;
    form->addRow(tr("IP address"), addressEdit_);
    form->addRow(tr("TCP port"), portSpin_);
    form->addRow(tr("Display name"), nameEdit_);
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(rememberCheck_);
    layout->addWidget(connectCheck_);
    layout->addWidget(errorLabel_);
    layout->addWidget(buttons_);

    connect(addressEdit_, &QLineEdit::textChanged, this, &AddDeviceDialog::validate);
    connect(buttons_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    validate();
}

QString AddDeviceDialog::address() const {
    return addressEdit_->text().trimmed();
}

quint16 AddDeviceDialog::port() const {
    return static_cast<quint16>(portSpin_->value());
}

QString AddDeviceDialog::displayName() const {
    return nameEdit_->text().trimmed();
}

bool AddDeviceDialog::rememberDevice() const {
    return rememberCheck_->isChecked();
}

bool AddDeviceDialog::connectImmediately() const {
    return connectCheck_->isChecked();
}

void AddDeviceDialog::validate() {
    const bool valid = !address().isEmpty() && !QHostAddress(address()).isNull();
    errorLabel_->setText(
        valid ? QString() : tr("Enter a valid IPv4 or IPv6 address.")
    );
    buttons_->button(QDialogButtonBox::Save)->setEnabled(valid);
}

