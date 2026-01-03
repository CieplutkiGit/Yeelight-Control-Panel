#include "MainWindow.h"

#include "dialogs/AddDeviceDialog.h"
#include "models/DeviceListModel.h"
#include "pages/ColorPage.h"
#include "pages/DashboardPage.h"
#include "pages/EffectsPage.h"
#include "pages/AutomationsPage.h"
#include "pages/DevicePage.h"
#include "pages/LogsPage.h"

#include <QAction>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(
    DeviceManager* manager,
    SettingsRepository* settings,
    AutomationEngine* automations,
    QWidget* parent
)
    : QMainWindow(parent)
    , manager_(manager)
    , deviceModel_(new DeviceListModel(manager, this))
    , proxyModel_(new QSortFilterProxyModel(this))
    , deviceListView_(new QListView(this))
    , statusLabel_(new QLabel(tr("No LAN devices found"), this))
    , selectedNameLabel_(new QLabel(tr("No device selected"), this))
    , selectedDetailsLabel_(new QLabel(this))
    , connectionBadge_(new QLabel(tr("Offline"), this))
    , powerOnButton_(new QPushButton(tr("On"), this))
    , powerOffButton_(new QPushButton(tr("Off"), this))
    , contentStack_(new QStackedWidget(this))
    , tabs_(new QTabWidget(this))
    , dashboardPage_(new DashboardPage(this))
    , colorPage_(new ColorPage(this))
    , effectsPage_(new EffectsPage(settings, this))
    , automationsPage_(new AutomationsPage(automations, manager, this))
    , devicePage_(new DevicePage(manager, this))
    , logsPage_(new LogsPage(settings, this)) {
    setWindowTitle(tr("Yeelight LAN"));
    resize(1180, 760);
    setMinimumSize(900, 600);

    auto* central = new QWidget(this);
    auto* outerLayout = new QHBoxLayout(central);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    auto* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setChildrenCollapsible(false);

    auto* sidebar = new QWidget(splitter);
    sidebar->setMinimumWidth(240);
    sidebar->setMaximumWidth(360);
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    auto* title = new QLabel(tr("Yeelight LAN"), sidebar);
    title->setObjectName(QStringLiteral("applicationTitle"));
    auto* localBadge = new QLabel(tr("Local network only"), sidebar);
    localBadge->setObjectName(QStringLiteral("localOnlyBadge"));
    auto* search = new QLineEdit(sidebar);
    search->setObjectName(QStringLiteral("deviceSearchEdit"));
    search->setPlaceholderText(tr("Search devices"));
    deviceListView_->setObjectName(QStringLiteral("deviceListView"));
    deviceListView_->setUniformItemSizes(true);
    auto* discoverButton = new QPushButton(tr("Discover"), sidebar);
    discoverButton->setObjectName(QStringLiteral("discoverButton"));
    auto* addButton = new QPushButton(tr("Add by IP"), sidebar);
    addButton->setObjectName(QStringLiteral("addDeviceButton"));
    auto* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(discoverButton);
    buttonRow->addWidget(addButton);
    sidebarLayout->addWidget(title);
    sidebarLayout->addWidget(localBadge);
    sidebarLayout->addWidget(search);
    sidebarLayout->addWidget(deviceListView_, 1);
    sidebarLayout->addLayout(buttonRow);
    sidebarLayout->addWidget(statusLabel_);

    proxyModel_->setSourceModel(deviceModel_);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel_->setFilterRole(Qt::DisplayRole);
    deviceListView_->setModel(proxyModel_);

    auto* mainContent = new QWidget(splitter);
    auto* mainLayout = new QVBoxLayout(mainContent);
    auto* header = new QWidget(mainContent);
    auto* headerLayout = new QHBoxLayout(header);
    auto* identityLayout = new QVBoxLayout;
    selectedNameLabel_->setObjectName(QStringLiteral("selectedDeviceName"));
    selectedDetailsLabel_->setObjectName(QStringLiteral("selectedDeviceDetails"));
    connectionBadge_->setObjectName(QStringLiteral("connectionBadge"));
    identityLayout->addWidget(selectedNameLabel_);
    identityLayout->addWidget(selectedDetailsLabel_);
    headerLayout->addLayout(identityLayout, 1);
    headerLayout->addWidget(connectionBadge_);
    auto* reconnectButton = new QPushButton(tr("Reconnect"), header);
    headerLayout->addWidget(reconnectButton);
    headerLayout->addWidget(powerOnButton_);
    headerLayout->addWidget(powerOffButton_);
    mainLayout->addWidget(header);

    auto* emptyPage = new QWidget(contentStack_);
    auto* emptyLayout = new QVBoxLayout(emptyPage);
    auto* emptyTitle = new QLabel(tr("No Yeelight LAN devices found"), emptyPage);
    emptyTitle->setObjectName(QStringLiteral("emptyStateTitle"));
    auto* instructions = new QLabel(
        tr("1. Connect the computer and Yeelight device to the same network.\n"
           "2. Enable LAN Control for the device.\n"
           "3. Select Discover.\n"
           "4. If discovery is blocked, add the device by IP address."),
        emptyPage
    );
    instructions->setWordWrap(true);
    auto* emptyButtons = new QHBoxLayout;
    auto* emptyDiscover = new QPushButton(tr("Discover"), emptyPage);
    auto* emptyAdd = new QPushButton(tr("Add by IP"), emptyPage);
    emptyButtons->addWidget(emptyDiscover);
    emptyButtons->addWidget(emptyAdd);
    emptyButtons->addStretch();
    emptyLayout->addStretch();
    emptyLayout->addWidget(emptyTitle);
    emptyLayout->addWidget(instructions);
    emptyLayout->addLayout(emptyButtons);
    emptyLayout->addStretch();

    tabs_->addTab(dashboardPage_, tr("Dashboard"));
    tabs_->addTab(colorPage_, tr("Color"));
    tabs_->addTab(effectsPage_, tr("Effects"));
    tabs_->addTab(automationsPage_, tr("Automations"));
    tabs_->addTab(devicePage_, tr("Device"));
    tabs_->addTab(logsPage_, tr("Logs"));
    contentStack_->addWidget(emptyPage);
    contentStack_->addWidget(tabs_);
    mainLayout->addWidget(contentStack_, 1);

    splitter->addWidget(sidebar);
    splitter->addWidget(mainContent);
    splitter->setSizes({280, 900});
    outerLayout->addWidget(splitter);
    setCentralWidget(central);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    auto* themeMenu = viewMenu->addMenu(tr("Theme"));
    for (const auto& pair : QList<QPair<QString, QString>>{
             {tr("System"), QStringLiteral("system")},
             {tr("Light"), QStringLiteral("light")},
             {tr("Dark"), QStringLiteral("dark")}
         }) {
        QAction* action = themeMenu->addAction(pair.first);
        connect(action, &QAction::triggered, this,
            [this, theme = pair.second] { emit themeRequested(theme); });
    }
    auto* developerAction = viewMenu->addAction(tr("Developer mode"));
    developerAction->setCheckable(true);
    developerAction->setChecked(
        settings != nullptr
            && settings->value(
                QStringLiteral("settings/developerMode"),
                false
            ).toBool()
    );
    connect(developerAction, &QAction::toggled,
        logsPage_, &LogsPage::setDeveloperMode);
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    connect(helpMenu->addAction(tr("About")), &QAction::triggered,
        this, &MainWindow::showAbout);

    connect(search, &QLineEdit::textChanged,
        proxyModel_, &QSortFilterProxyModel::setFilterFixedString);
    connect(deviceListView_->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &MainWindow::selectDevice);
    connect(discoverButton, &QPushButton::clicked, manager_, &DeviceManager::startDiscovery);
    connect(emptyDiscover, &QPushButton::clicked, manager_, &DeviceManager::startDiscovery);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addManualDevice);
    connect(emptyAdd, &QPushButton::clicked, this, &MainWindow::addManualDevice);
    connect(devicePage_, &DevicePage::addDeviceRequested,
        this, &MainWindow::addManualDevice);
    connect(reconnectButton, &QPushButton::clicked, this, [this] {
        if (selectedDevice_ != nullptr) {
            selectedDevice_->connectDevice();
        }
    });
    connect(powerOnButton_, &QPushButton::clicked, this, [this] {
        if (selectedDevice_ != nullptr) {
            selectedDevice_->setPower(true);
        }
    });
    connect(powerOffButton_, &QPushButton::clicked, this, [this] {
        if (selectedDevice_ != nullptr) {
            selectedDevice_->setPower(false);
        }
    });
    connect(manager_, &DeviceManager::deviceAdded, this,
        [this](DeviceController*) { updateStatusText(); });
    connect(manager_, &DeviceManager::deviceRemoved, this,
        [this](const QString&) { updateStatusText(); });
    connect(manager_, &DeviceManager::discoveryStateChanged, this,
        [this](bool active) {
            statusLabel_->setText(active ? tr("Discovering…") : tr("Discovery finished"));
        });
    connect(manager_, &DeviceManager::errorOccurred, this,
        [this](const QString& message) { statusBar()->showMessage(message, 6000); });

    updateSelection(nullptr);
    updateStatusText();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    emit windowClosing(saveGeometry());
    QMainWindow::closeEvent(event);
}

QWidget* MainWindow::makeInformationalPage(
    const QString& title,
    const QString& text
) {
    auto* page = new QWidget(tabs_);
    auto* layout = new QVBoxLayout(page);
    auto* titleLabel = new QLabel(title, page);
    auto* textLabel = new QLabel(text, page);
    textLabel->setWordWrap(true);
    layout->addWidget(titleLabel);
    layout->addWidget(textLabel);
    layout->addStretch();
    return page;
}

void MainWindow::selectDevice(const QModelIndex& proxyIndex) {
    const QModelIndex sourceIndex = proxyModel_->mapToSource(proxyIndex);
    updateSelection(deviceModel_->controllerAt(sourceIndex.row()));
}

void MainWindow::updateSelection(DeviceController* controller) {
    if (selectedDevice_ != nullptr) {
        disconnect(selectedDevice_, nullptr, this, nullptr);
    }
    selectedDevice_ = controller;
    dashboardPage_->setDevice(selectedDevice_);
    colorPage_->setDevice(selectedDevice_);
    effectsPage_->setDevice(selectedDevice_);
    devicePage_->setDevice(selectedDevice_);
    logsPage_->setDevice(selectedDevice_);
    const bool selected = selectedDevice_ != nullptr;
    contentStack_->setCurrentIndex(selected ? 1 : 0);
    powerOnButton_->setEnabled(selected);
    powerOffButton_->setEnabled(selected);
    if (!selected) {
        selectedNameLabel_->setText(tr("No device selected"));
        selectedDetailsLabel_->clear();
        connectionBadge_->setText(tr("Offline"));
        return;
    }

    const auto refreshHeader = [this] {
        const DeviceInfo info = selectedDevice_->info();
        const DeviceState state = selectedDevice_->state();
        selectedNameLabel_->setText(
            info.name.isEmpty() ? info.ipAddress : info.name
        );
        selectedDetailsLabel_->setText(
            QStringLiteral("%1 · %2").arg(info.model, info.ipAddress)
        );
        connectionBadge_->setText(state.reachable ? tr("Online") : tr("Offline"));
        powerOnButton_->setEnabled(info.capabilities.supports(QStringLiteral("set_power")));
        powerOffButton_->setEnabled(info.capabilities.supports(QStringLiteral("set_power")));
    };
    connect(selectedDevice_, &DeviceController::infoChanged, this, refreshHeader);
    connect(selectedDevice_, &DeviceController::stateChanged, this, refreshHeader);
    refreshHeader();
}

void MainWindow::updateStatusText() {
    const int count = manager_->devices().size();
    statusLabel_->setText(
        count == 0 ? tr("No LAN devices found") : tr("%n device(s) found", "", count)
    );
}

void MainWindow::addManualDevice() {
    AddDeviceDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    if (manager_->addManualDevice(
            dialog.address(),
            dialog.port(),
            dialog.displayName(),
            dialog.rememberDevice()
        )
        && dialog.connectImmediately()) {
        const QString normalizedAddress = QHostAddress(dialog.address()).toString();
        auto* controller = manager_->device(
            QStringLiteral("%1:%2").arg(normalizedAddress).arg(dialog.port())
        );
        if (controller != nullptr) {
            controller->connectDevice();
        }
    }
}

void MainWindow::showAbout() {
    QMessageBox::about(
        this,
        tr("About Yeelight LAN"),
        tr("Yeelight LAN communicates directly with compatible devices on your local "
           "network. It does not use a cloud control service.")
    );
}
