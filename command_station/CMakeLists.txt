cmake_minimum_required(VERSION 3.5)
project(command_station)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif()

file(GLOB_RECURSE serial_src src/serial/*.cpp)
add_library(serial STATIC ${serial_src})
target_include_directories(serial PUBLIC include)

if (WIN32)
    target_link_libraries(serial PUBLIC setupapi)
endif ()

file(GLOB midi_src src/midi/*.cpp)
if (WIN32)
    list(APPEND midi_src
            src/midi/midi_input.cpp
            include/midi/midi_input.h
            include/midi/midi_file_player.h)
    add_library(midi STATIC ${midi_src})
    target_compile_definitions(midi PUBLIC __WINDOWS_MM__)
elseif (UNIX AND NOT APPLE)
    list(APPEND midi_src
            src/midi/midi_input.cpp
            include/midi/midi_input.h
            include/midi/midi_file_player.h)
    add_library(midi STATIC ${midi_src})
    target_link_libraries(midi PUBLIC pthread asound)
    target_compile_definitions(midi PUBLIC __LINUX_ALSA__)
elseif (APPLE)
    list(APPEND midi_src
            src/midi/midi_input.cpp
            include/midi/midi_input.h
            include/midi/midi_file_player.h)
    add_library(midi STATIC ${midi_src})
    target_link_libraries(midi PUBLIC
            "-framework CoreServices"
            "-framework CoreAudio"
            "-framework CoreMidi"
            "-framework CoreFoundation"
            ${IOKIT_LIBRARY}
            ${FOUNDATION_LIBRARY})
    target_compile_definitions(midi PUBLIC __MACOSX_CORE__)
endif ()
target_include_directories(midi PUBLIC include)
target_link_libraries(midi PUBLIC Qt5::Core Qt5::Widgets)

set(command_station_src
        src/command_station.cpp
        src/visualizer.cpp
        src/main_widget.cpp
        include/main_widget.h
        include/visualizer.h
        rc/command_station.qrc)
# win32 has no effect on osx/linux, so we can use it here without ifdefs
add_executable(command_station WIN32 ${command_station_src})
target_include_directories(command_station PUBLIC include)
target_link_libraries(command_station PUBLIC midi serial Qt5::Core Qt5::Widgets Qt5::Multimedia)
