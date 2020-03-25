cmake_minimum_required(VERSION 3.5)
project(glowsuit)

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif ()

if (ARDUINO_BOARD)
    add_subdirectory(arduino)
else ()
    add_subdirectory(command_station)
endif ()
