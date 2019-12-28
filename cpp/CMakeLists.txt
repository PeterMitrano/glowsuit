cmake_minimum_required(VERSION 3.10)
project(command_station)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE serial_src serial/*.cpp)
add_library(serial SHARED ${serial_src})
target_include_directories(serial PUBLIC include)

file(GLOB_RECURSE midi_src midi/*.cpp)
add_library(midi SHARED ${midi_src})
target_include_directories(midi PUBLIC include)

file(GLOB main_src *.cpp)
add_executable(command_station ${main_src})
target_include_directories(command_station PUBLIC include)
target_link_libraries(command_station midi serial)
