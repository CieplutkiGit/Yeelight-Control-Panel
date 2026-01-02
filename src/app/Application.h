#pragma once

#include "../core/device/DeviceManager.h"
#include "../core/persistence/SettingsRepository.h"

#include <QApplication>

#include <memory>

class MainWindow;

class Application final : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    int run();

private:
    void applyTheme(const QString& theme);

    SettingsRepository settings_;
    DeviceManager deviceManager_;
    std::unique_ptr<MainWindow> mainWindow_;
};

