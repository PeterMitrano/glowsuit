cmake_minimum_required(VERSION 3.10)
project(command_station)

set(CMAKE_CXX_STANDARD 17)
add_executable(command_station main.cpp)
target_include_directories(command_station PUBLIC include)
