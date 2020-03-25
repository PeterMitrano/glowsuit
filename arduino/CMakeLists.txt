add_executable(suits
        suits/suit_main.cpp
        suits/scoped_suit_main.cpp
        suits/suit_1.cpp)
target_link_arduino_libraries(suits PRIVATE core XBee)
target_include_directories(suits PUBLIC suits/)
target_enable_arduino_upload(suits)