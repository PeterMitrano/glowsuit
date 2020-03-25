add_library(suits suits/suit_main.cpp suits/suit_1.cpp)
target_link_arduino_libraries(suits PRIVATE core XBee)