project(command_station)

set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1 ")
set(CMAKE_CXX_FLAGS_RELEASE "/MT /Zi /Ob0 /Od /RTC1 ")

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
set(MIDI_LIBS)
if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_library(midi STATIC ${midi_src})
    target_compile_definitions(midi PUBLIC __WINDOWS_MM__)
    list(APPEND MIDI_LIBS winmm)
elseif (UNIX AND NOT APPLE)
    add_library(midi STATIC ${midi_src})
    list(APPEND MIDI_LIBS pthread asound)
    target_compile_definitions(midi PUBLIC __LINUX_ALSA__)
elseif (APPLE)
    add_library(midi STATIC ${midi_src})
    list(APPEND MIDI_LIBS 
            "-framework CoreServices"
            "-framework CoreAudio"
            "-framework CoreMidi"
            "-framework CoreFoundation"
            ${IOKIT_LIBRARY}
            ${FOUNDATION_LIBRARY})
    target_compile_definitions(midi PUBLIC __MACOSX_CORE__)
endif ()

target_include_directories(midi PUBLIC include)
target_link_libraries(midi PUBLIC Qt5::Core Qt5::Widgets Qt5::Multimedia ${MIDI_LIBS})

set(command_station_src
        src/XBee.cpp
        ../arduino/suits/suit_command.cpp
        src/live_midi_input.cpp
        src/all_suits_choreo.cpp
        src/common.cpp
        src/command_station.cpp
        src/visualizer.cpp
        src/main_widget.cpp
        include/live_midi_input.h
        include/main_widget.h
        include/visualizer.h
        rc/command_station.qrc)

# win32 has no effect on osx/linux, so we can use it here without ifdefs
add_executable(command_station WIN32 ${command_station_src})
target_include_directories(command_station PUBLIC include include/mock_arduino ../arduino/suits/)
target_link_libraries(command_station PUBLIC midi serial Qt5::Core Qt5::Widgets Qt5::Multimedia)
