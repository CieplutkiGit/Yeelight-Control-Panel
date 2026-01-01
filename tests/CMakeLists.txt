add_executable(test_command_serialization test_command_serialization.cpp)
target_link_libraries(test_command_serialization PRIVATE
    yeelight_core
    Qt6::Test
)
add_test(NAME command_serialization COMMAND test_command_serialization)
