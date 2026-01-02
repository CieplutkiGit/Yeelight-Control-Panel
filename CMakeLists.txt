cmake_minimum_required(VERSION 3.21)

project(YeelightControlPanel
    VERSION 2.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Test
)

add_library(yeelight_core STATIC
    src/core/model/DeviceCapabilities.h
    src/core/model/DeviceInfo.h
    src/core/model/DeviceState.h
    src/core/model/EffectPreset.h
    src/core/model/ScheduledAction.h
    src/core/protocol/YeelightCommand.cpp
    src/core/protocol/YeelightCommand.h
    src/core/protocol/YeelightDiscoveryParser.cpp
    src/core/protocol/YeelightDiscoveryParser.h
    src/core/network/DiscoveryService.cpp
    src/core/network/DiscoveryService.h
    src/core/network/YeelightConnection.cpp
    src/core/network/YeelightConnection.h
    src/core/protocol/YeelightMessageParser.cpp
    src/core/protocol/YeelightMessageParser.h
    src/core/device/DeviceController.cpp
    src/core/device/DeviceController.h
    src/core/device/DeviceManager.cpp
    src/core/device/DeviceManager.h
    src/core/persistence/SettingsRepository.cpp
    src/core/persistence/SettingsRepository.h
    src/core/logging/AppLogger.cpp
    src/core/logging/AppLogger.h
)
target_include_directories(yeelight_core PUBLIC src)
target_link_libraries(yeelight_core PUBLIC Qt6::Core Qt6::Gui Qt6::Network)

add_executable(YeelightControlPanel
    src/main.cpp
    src/app/Application.cpp
    src/app/Application.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/models/DeviceListModel.cpp
    src/ui/models/DeviceListModel.h
    src/ui/dialogs/AddDeviceDialog.cpp
    src/ui/dialogs/AddDeviceDialog.h
    src/ui/pages/DashboardPage.cpp
    src/ui/pages/DashboardPage.h
    src/ui/pages/ColorPage.cpp
    src/ui/pages/ColorPage.h
    src/ui/widgets/ColorPreviewWidget.cpp
    src/ui/widgets/ColorPreviewWidget.h
    resources/resources.qrc
)

target_include_directories(YeelightControlPanel PRIVATE src)
target_link_libraries(YeelightControlPanel PRIVATE
    yeelight_core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if(MSVC)
    target_compile_options(YeelightControlPanel PRIVATE /W4 /permissive-)
else()
    target_compile_options(YeelightControlPanel PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
