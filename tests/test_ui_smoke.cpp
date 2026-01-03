#include "../src/core/device/DeviceController.h"
#include "../src/core/device/DeviceManager.h"
#include "../src/ui/MainWindow.h"
#include "../src/ui/pages/ColorPage.h"

#include <QLabel>
#include <QSlider>
#include <QtTest>

class UiSmokeTest final : public QObject {
    Q_OBJECT

private slots:
    void windowOpensWithEmptyState();
    void unsupportedControlsAreDisabled();
    void stateUpdatesDoNotSendCommands();
};

void UiSmokeTest::windowOpensWithEmptyState() {
    DeviceManager manager;
    MainWindow window(&manager, nullptr, nullptr);
    window.show();
    QTest::qWait(20);
    auto* title = window.findChild<QLabel*>(QStringLiteral("emptyStateTitle"));
    QVERIFY(title != nullptr);
    QVERIFY(title->isVisible());
    QVERIFY(window.minimumWidth() >= 900);
}

void UiSmokeTest::unsupportedControlsAreDisabled() {
    DeviceInfo info;
    info.ipAddress = QStringLiteral("127.0.0.1");
    DeviceController controller(info);
    ColorPage page(nullptr);
    page.setDevice(&controller);
    QVERIFY(!page.findChild<QSlider*>(
        QStringLiteral("brightnessSlider"))->isEnabled());
    QVERIFY(!page.findChild<QSlider*>(
        QStringLiteral("temperatureSlider"))->isEnabled());
}

void UiSmokeTest::stateUpdatesDoNotSendCommands() {
    DeviceInfo info;
    info.ipAddress = QStringLiteral("127.0.0.1");
    info.capabilities.methods = {
        QStringLiteral("set_bright"),
        QStringLiteral("set_ct_abx")
    };
    DeviceState state;
    state.brightness = 40;
    state.colorTemperature = 3500;
    DeviceController controller(info, state);
    QSignalSpy unsupported(&controller, &DeviceController::unsupportedOperation);
    ColorPage page(nullptr);
    page.setDevice(&controller);
    QCOMPARE(
        page.findChild<QSlider*>(QStringLiteral("brightnessSlider"))->value(),
        40
    );
    QCOMPARE(unsupported.size(), 0);
}

QTEST_MAIN(UiSmokeTest)
#include "test_ui_smoke.moc"
