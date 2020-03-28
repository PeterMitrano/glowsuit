project(command_station)

find_package(Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE serial_src src/serial/*.cpp)
add_library(serial STATIC ${serial_src})
target_include_directories(serial PUBLIC include)

if (WIN32)
    target_link_libraries(serial PUBLIC setupapi)
endif ()

file(GLOB midi_src src/midi/*.cpp)
if (WIN32)
    list(APPEND midi_src
            src/midi/live_midi_input.cpp
            include/midi/live_midi_input.h)
    add_library(midi STATIC ${midi_src})
    target_compile_definitions(midi PUBLIC __WINDOWS_MM__)
elseif (UNIX AND NOT APPLE)
    list(APPEND midi_src
            src/midi/live_midi_input.cpp
            include/midi/live_midi_input.h)
    add_library(midi STATIC ${midi_src})
    target_link_libraries(midi PUBLIC pthread asound)
    target_compile_definitions(midi PUBLIC __LINUX_ALSA__)
elseif (APPLE)
    list(APPEND midi_src
            src/midi/live_midi_input.cpp
            include/midi/live_midi_input.h)
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
target_link_libraries(midi PUBLIC Qt5::Core Qt5::Widgets Qt5::Multimedia)

set(command_station_src
        ../arduino/suits/suit_program.cpp
        ../arduino/suits/suit_common.cpp
        src/Arduino.cpp
        src/XBee.cpp
        src/suit_dispatcher.cpp
        src/all_suits_choreo.cpp
        src/common.cpp
        src/command_station.cpp
        src/suit_worker.cpp
        src/visualizer.cpp
        src/main_widget.cpp
        include/main_widget.h
        include/visualizer.h
        include/suit_worker.h
        rc/command_station.qrc)

# win32 has no effect on osx/linux, so we can use it here without ifdefs
add_executable(command_station WIN32 ${command_station_src})
target_include_directories(command_station PUBLIC include include/mock_arduino ../arduino/suits/)
target_link_libraries(command_station PUBLIC midi serial Qt5::Core Qt5::Widgets Qt5::Multimedia)
