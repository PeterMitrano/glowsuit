add_executable(suits
        suits/main.cpp
        suits/suit_program.cpp
        suits/suit_common.cpp
        suits/suit_1.cpp)
target_link_arduino_libraries(suits PRIVATE core XBee ArduinoSTL)
target_include_directories(suits PUBLIC suits ${CMAKE_CURRENT_SOURCE_DIR})
target_enable_arduino_upload(suits)