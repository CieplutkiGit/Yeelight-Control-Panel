add_executable(test_command_serialization test_command_serialization.cpp)
target_link_libraries(test_command_serialization PRIVATE
    yeelight_core
    Qt6::Test
)
add_test(NAME command_serialization COMMAND test_command_serialization)

add_executable(test_discovery_parser test_discovery_parser.cpp)
target_link_libraries(test_discovery_parser PRIVATE
    yeelight_core
    Qt6::Test
)
add_test(NAME discovery_parser COMMAND test_discovery_parser)

add_executable(test_message_parser test_message_parser.cpp)
target_link_libraries(test_message_parser PRIVATE yeelight_core Qt6::Test)
add_test(NAME message_parser COMMAND test_message_parser)

add_executable(test_connection
    test_connection.cpp
    mocks/MockYeelightServer.cpp
    mocks/MockYeelightServer.h
)
target_link_libraries(test_connection PRIVATE yeelight_core Qt6::Test)
add_test(NAME connection COMMAND test_connection)

add_executable(test_device_manager test_device_manager.cpp)
target_link_libraries(test_device_manager PRIVATE yeelight_core Qt6::Test)
add_test(NAME device_manager COMMAND test_device_manager)

add_executable(test_settings_repository test_settings_repository.cpp)
target_link_libraries(test_settings_repository PRIVATE yeelight_core Qt6::Test)
add_test(NAME settings_repository COMMAND test_settings_repository)

add_executable(test_ui_smoke
    test_ui_smoke.cpp
    ../src/ui/MainWindow.cpp
    ../src/ui/MainWindow.h
    ../src/ui/pages/DashboardPage.cpp
    ../src/ui/pages/DashboardPage.h
    ../src/ui/pages/ColorPage.cpp
    ../src/ui/pages/ColorPage.h
    ../src/ui/pages/EffectsPage.cpp
    ../src/ui/pages/EffectsPage.h
    ../src/ui/dialogs/EffectEditorDialog.cpp
    ../src/ui/dialogs/EffectEditorDialog.h
    ../src/ui/pages/AutomationsPage.cpp
    ../src/ui/pages/AutomationsPage.h
    ../src/ui/dialogs/ScheduleEditorDialog.cpp
    ../src/ui/dialogs/ScheduleEditorDialog.h
    ../src/ui/pages/DevicePage.cpp
    ../src/ui/pages/DevicePage.h
    ../src/ui/pages/LogsPage.cpp
    ../src/ui/pages/LogsPage.h
    ../src/ui/widgets/SegmentPreviewWidget.cpp
    ../src/ui/widgets/SegmentPreviewWidget.h
    ../src/ui/widgets/ConnectionBadge.cpp
    ../src/ui/widgets/ConnectionBadge.h
    ../src/ui/widgets/CardWidget.cpp
    ../src/ui/widgets/CardWidget.h
    ../src/ui/widgets/PowerToggle.cpp
    ../src/ui/widgets/PowerToggle.h
    ../src/ui/widgets/DeviceListDelegate.cpp
    ../src/ui/widgets/DeviceListDelegate.h
    ../src/ui/widgets/EffectTimelineWidget.cpp
    ../src/ui/widgets/EffectTimelineWidget.h
    ../src/ui/widgets/ColorPreviewWidget.cpp
    ../src/ui/widgets/ColorPreviewWidget.h
    ../src/ui/dialogs/AddDeviceDialog.cpp
    ../src/ui/dialogs/AddDeviceDialog.h
    ../src/ui/models/DeviceListModel.cpp
    ../src/ui/models/DeviceListModel.h
)
target_link_libraries(test_ui_smoke PRIVATE
    yeelight_core
    Qt6::Widgets
    Qt6::Test
)
add_test(NAME ui_smoke COMMAND test_ui_smoke)
set_tests_properties(ui_smoke PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(test_effects
    test_effects.cpp
    ../src/ui/pages/EffectsPage.cpp
    ../src/ui/pages/EffectsPage.h
    ../src/ui/dialogs/EffectEditorDialog.cpp
    ../src/ui/dialogs/EffectEditorDialog.h
    ../src/ui/widgets/EffectTimelineWidget.cpp
    ../src/ui/widgets/EffectTimelineWidget.h
)
target_link_libraries(test_effects PRIVATE yeelight_core Qt6::Widgets Qt6::Test)
add_test(NAME effects COMMAND test_effects)

add_executable(test_automation_engine test_automation_engine.cpp)
target_link_libraries(test_automation_engine PRIVATE yeelight_core Qt6::Test)
add_test(NAME automation_engine COMMAND test_automation_engine)

add_test(
    NAME source_policy
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/check_lan_only.cmake
)
