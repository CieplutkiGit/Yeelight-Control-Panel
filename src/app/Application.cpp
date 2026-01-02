#include "Application.h"

#include "../ui/MainWindow.h"

#include <QFile>
#include <QStyleHints>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv) {
    setApplicationName(QStringLiteral("Yeelight LAN"));
    setApplicationDisplayName(QStringLiteral("Yeelight LAN"));
    setApplicationVersion(QStringLiteral("2.0.0"));
    setOrganizationName(QStringLiteral("CieplutkiGit"));
}

int Application::run() {
    const QString theme = settings_.value(
        QStringLiteral("ui/theme"),
        QStringLiteral("system")
    ).toString();
    applyTheme(theme);

    mainWindow_ = std::make_unique<MainWindow>(&deviceManager_, &settings_);
    mainWindow_->restoreGeometry(
        settings_.value(QStringLiteral("ui/windowGeometry")).toByteArray()
    );
    connect(mainWindow_.get(), &MainWindow::themeRequested, this,
        [this](const QString& selectedTheme) {
            settings_.setValue(QStringLiteral("ui/theme"), selectedTheme);
            applyTheme(selectedTheme);
        });
    connect(mainWindow_.get(), &MainWindow::windowClosing, this,
        [this](const QByteArray& geometry) {
            settings_.setValue(QStringLiteral("ui/windowGeometry"), geometry);
            settings_.sync();
        });
    mainWindow_->show();
    return exec();
}

void Application::applyTheme(const QString& theme) {
    QString effectiveTheme = theme;
    if (effectiveTheme == QStringLiteral("system")) {
        const auto scheme = styleHints()->colorScheme();
        effectiveTheme = scheme == Qt::ColorScheme::Dark
            ? QStringLiteral("dark")
            : QStringLiteral("light");
    }
    QFile file(QStringLiteral(":/styles/%1.qss").arg(effectiveTheme));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(file.readAll()));
    }
}
