#include "MainWindow.h"

#include "dialogs/AddDeviceDialog.h"
#include "models/DeviceListModel.h"
#include "pages/ColorPage.h"
#include "pages/DashboardPage.h"
#include "pages/EffectsPage.h"
#include "pages/AutomationsPage.h"
#include "pages/DevicePage.h"
#include "pages/LogsPage.h"
#include "widgets/CardWidget.h"
#include "widgets/ConnectionBadge.h"
#include "widgets/DeviceListDelegate.h"

#include <QAction>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QIcon>
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
#include <QVBoxLayout>

MainWindow::MainWindow(
    DeviceManager* manager,
    SettingsRepository* settings,
    AutomationEngine* automations,
    QWidget* parent
)
    : QMainWindow(parent)
    , manager_(manager)
    , settings_(settings)
    , deviceModel_(new DeviceListModel(manager, settings, this))
    , proxyModel_(new QSortFilterProxyModel(this))
    , deviceListView_(new QListView(this))
    , statusLabel_(new QLabel(tr("No LAN devices found"), this))
    , selectedNameLabel_(new QLabel(tr("No device selected"), this))
    , selectedDetailsLabel_(new QLabel(this))
    , connectionBadge_(new ConnectionBadge(this))
    , powerOnButton_(new QPushButton(tr("On"), this))
    , powerOffButton_(new QPushButton(tr("Off"), this))
    , favoriteButton_(new QPushButton(tr("Favorite"), this))
    , contentStack_(new QStackedWidget(this))
    , navigationGroup_(new QButtonGroup(this))
    , splitter_(new QSplitter(Qt::Horizontal, this))
    , dashboardPage_(new DashboardPage(this))
    , colorPage_(new ColorPage(settings, this))
    , effectsPage_(new EffectsPage(settings, this))
    , automationsPage_(new AutomationsPage(automations, manager, this))
    , devicePage_(new DevicePage(manager, this))
    , logsPage_(new LogsPage(settings, this)) {
    setWindowTitle(tr("Yeelight LAN"));
    resize(1180, 760);
    setMinimumSize(900, 600);

    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("mainWindowSurface"));
    auto* outerLayout = new QHBoxLayout(central);
    outerLayout->setContentsMargins(14, 14, 14, 14);
    outerLayout->setSpacing(14);
    splitter_->setChildrenCollapsible(false);

    auto* sidebar = new QFrame(splitter_);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setMinimumWidth(240);
    sidebar->setMaximumWidth(360);
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(18, 18, 18, 18);
    sidebarLayout->setSpacing(12);
    auto* title = new QLabel(tr("Yeelight LAN"), sidebar);
    title->setObjectName(QStringLiteral("applicationTitle"));
    auto* localCard = new CardWidget(tr("Local network"), sidebar);
    auto* localBadge = new QLabel(tr("Local network only"), localCard);
    localBadge->setObjectName(QStringLiteral("localOnlyBadge"));
    localCard->contentLayout()->addWidget(localBadge);
    auto* search = new QLineEdit(sidebar);
    search->setObjectName(QStringLiteral("deviceSearchEdit"));
    search->setPlaceholderText(tr("Search devices"));
    deviceListView_->setObjectName(QStringLiteral("deviceListView"));
    deviceListView_->setUniformItemSizes(true);
    deviceListView_->setItemDelegate(new DeviceListDelegate(deviceListView_));
    auto* discoverButton = new QPushButton(tr("Discover"), sidebar);
    discoverButton->setObjectName(QStringLiteral("discoverButton"));
    auto* addButton = new QPushButton(tr("Add by IP"), sidebar);
    addButton->setObjectName(QStringLiteral("addDeviceButton"));
    auto* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(discoverButton);
    buttonRow->addWidget(addButton);
    auto* navigationTitle = new QLabel(tr("CONTROL"), sidebar);
    navigationTitle->setObjectName(QStringLiteral("sectionLabel"));
    auto* navigationLayout = new QVBoxLayout;
    navigationLayout->setSpacing(4);
    navigationGroup_->setExclusive(true);
    const QList<QPair<QString, QString>> navigationItems{
        {tr("Dashboard"), QStringLiteral(":/icons/dashboard.svg")},
        {tr("Color"), QStringLiteral(":/icons/color.svg")},
        {tr("Effects"), QStringLiteral(":/icons/effects.svg")},
        {tr("Automations"), QStringLiteral(":/icons/automations.svg")},
        {tr("Device"), QStringLiteral(":/icons/device.svg")},
        {tr("Logs"), QStringLiteral(":/icons/logs.svg")}
    };
    for (int index = 0; index < navigationItems.size(); ++index) {
        auto* button = new QPushButton(navigationItems.at(index).first, sidebar);
        button->setObjectName(QStringLiteral("navigationButton"));
        button->setCheckable(true);
        button->setIcon(QIcon(navigationItems.at(index).second));
        button->setIconSize(QSize(18, 18));
        navigationGroup_->addButton(button, index);
        navigationLayout->addWidget(button);
        if (index == 0) {
            button->setChecked(true);
        }
    }
    sidebarLayout->addWidget(title);
    sidebarLayout->addWidget(localCard);
    sidebarLayout->addWidget(search);
    sidebarLayout->addWidget(deviceListView_, 1);
    sidebarLayout->addLayout(buttonRow);
    sidebarLayout->addWidget(navigationTitle);
    sidebarLayout->addLayout(navigationLayout);
    sidebarLayout->addWidget(statusLabel_);

    proxyModel_->setSourceModel(deviceModel_);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel_->setFilterRole(Qt::DisplayRole);
    deviceListView_->setModel(proxyModel_);

    auto* mainContent = new QFrame(splitter_);
    mainContent->setObjectName(QStringLiteral("contentArea"));
    auto* mainLayout = new QVBoxLayout(mainContent);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(14);
    auto* header = new CardWidget({}, mainContent);
    auto* headerRow = new QHBoxLayout;
    auto* identityLayout = new QVBoxLayout;
    selectedNameLabel_->setObjectName(QStringLiteral("selectedDeviceName"));
    selectedDetailsLabel_->setObjectName(QStringLiteral("selectedDeviceDetails"));
    identityLayout->addWidget(selectedNameLabel_);
    identityLayout->addWidget(selectedDetailsLabel_);
    headerRow->addLayout(identityLayout, 1);
    headerRow->addWidget(connectionBadge_);
    favoriteButton_->setCheckable(true);
    headerRow->addWidget(favoriteButton_);
    auto* reconnectButton = new QPushButton(tr("Reconnect"), header);
    headerRow->addWidget(reconnectButton);
    headerRow->addWidget(powerOnButton_);
    headerRow->addWidget(powerOffButton_);
    header->contentLayout()->addLayout(headerRow);
    mainLayout->addWidget(header);

    auto* emptyPage = new CardWidget({}, contentStack_);
    auto* emptyLayout = emptyPage->contentLayout();
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

    contentStack_->addWidget(emptyPage);
    contentStack_->addWidget(dashboardPage_);
    contentStack_->addWidget(colorPage_);
    contentStack_->addWidget(effectsPage_);
    contentStack_->addWidget(automationsPage_);
    contentStack_->addWidget(devicePage_);
    contentStack_->addWidget(logsPage_);
    mainLayout->addWidget(contentStack_, 1);

    splitter_->addWidget(sidebar);
    splitter_->addWidget(mainContent);
    splitter_->setSizes({280, 900});
    if (settings_ != nullptr) {
        splitter_->restoreState(
            settings_->value(QStringLiteral("ui/splitterState")).toByteArray()
        );
    }
    outerLayout->addWidget(splitter_);
    setCentralWidget(central);

    connect(navigationGroup_, &QButtonGroup::idClicked, this, [this](int id) {
        contentStack_->setCurrentIndex(id + 1);
    });

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
    connect(favoriteButton_, &QPushButton::clicked, this, [this](bool favorite) {
        if (selectedDevice_ == nullptr || settings_ == nullptr) {
            return;
        }
        QStringList favorites = settings_->value(
            QStringLiteral("devices/favorites")
        ).toStringList();
        const QString id = selectedDevice_->info().stableId();
        favorites.removeAll(id);
        if (favorite) {
            favorites.append(id);
        }
        settings_->setValue(QStringLiteral("devices/favorites"), favorites);
        deviceModel_->refreshAll();
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
    if (settings_ != nullptr) {
        const QString selectedId = settings_->value(
            QStringLiteral("ui/lastSelectedDevice")
        ).toString();
        for (int row = 0; row < deviceModel_->rowCount(); ++row) {
            const QModelIndex sourceIndex = deviceModel_->index(row);
            if (sourceIndex.data(DeviceListModel::StableIdRole).toString() == selectedId) {
                deviceListView_->setCurrentIndex(proxyModel_->mapFromSource(sourceIndex));
                break;
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (settings_ != nullptr) {
        settings_->setValue(QStringLiteral("ui/splitterState"), splitter_->saveState());
        settings_->sync();
    }
    emit windowClosing(saveGeometry());
    QMainWindow::closeEvent(event);
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
    if (selectedDevice_ != nullptr && settings_ != nullptr) {
        settings_->setValue(
            QStringLiteral("ui/lastSelectedDevice"),
            selectedDevice_->info().stableId()
        );
    }
    dashboardPage_->setDevice(selectedDevice_);
    colorPage_->setDevice(selectedDevice_);
    effectsPage_->setDevice(selectedDevice_);
    devicePage_->setDevice(selectedDevice_);
    logsPage_->setDevice(selectedDevice_);
    const bool selected = selectedDevice_ != nullptr;
    contentStack_->setCurrentIndex(selected ? 1 : 0);
    for (auto* button : navigationGroup_->buttons()) {
        button->setEnabled(selected);
    }
    powerOnButton_->setEnabled(selected);
    powerOffButton_->setEnabled(selected);
    favoriteButton_->setEnabled(selected && settings_ != nullptr);
    if (!selected) {
        selectedNameLabel_->setText(tr("No device selected"));
        selectedDetailsLabel_->clear();
        connectionBadge_->setOnline(false);
        favoriteButton_->setChecked(false);
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
        connectionBadge_->setOnline(state.reachable);
        favoriteButton_->setChecked(
            settings_ != nullptr
                && settings_->value(QStringLiteral("devices/favorites"))
                    .toStringList().contains(info.stableId())
        );
        powerOnButton_->setEnabled(info.capabilities.supports(QStringLiteral("set_power")));
        powerOffButton_->setEnabled(info.capabilities.supports(QStringLiteral("set_power")));
    };
    connect(selectedDevice_, &DeviceController::infoChanged, this, refreshHeader);
    connect(selectedDevice_, &DeviceController::stateChanged, this, refreshHeader);
    refreshHeader();
}

void MainWindow::updateStatusText() {
    const int count = static_cast<int>(manager_->devices().size());
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
