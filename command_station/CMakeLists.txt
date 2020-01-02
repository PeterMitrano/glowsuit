cmake_minimum_required(VERSION 3.5)
project(command_station)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

file(GLOB_RECURSE serial_src src/serial/*.cpp)
if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	add_library(serial STATIC ${serial_src})
else()
	add_library(serial SHARED ${serial_src})
endif()
target_include_directories(serial PUBLIC include)

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	target_link_libraries(serial PUBLIC setupapi)
endif()

file(GLOB_RECURSE midi_src src/midi/*.cpp)
list(APPEND midi_src include/midi/midi_input.h)
if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	add_library(midi STATIC ${midi_src})
else()
	add_library(midi SHARED ${midi_src})
endif()
target_include_directories(midi PUBLIC include)
target_link_libraries(midi PUBLIC Qt5::Core)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message("Detected Linux...")
    set(audio_src src/audio/play_music-linux.cpp)
    add_library(audio SHARED ${audio_src})
    target_include_directories(audio PUBLIC include)
    target_link_libraries(audio PUBLIC asound pthread)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    message("Detected Windows...")
    set(audio_src src/audio/play_music-win.cpp)
    add_library(audio STATIC ${audio_src} include/audio/play_music.h)
    target_include_directories(audio PUBLIC include)
    target_link_libraries(audio PUBLIC winmm Qt5::Core)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    message("Detected Mac...")
endif ()

# TODO: ifdefs so this works on linux too
add_executable(command_station
				WIN32
				src/command_station.cpp
				src/visualizer.cpp
				src/main_widget.cpp
				include/main_widget.h
				include/visualizer.h
				rc/command_station.qrc)
target_include_directories(command_station PUBLIC include)
target_link_libraries(command_station PUBLIC audio midi serial Qt5::Core Qt5::Widgets)

add_executable(test_win_audio src/test_win_audio.cpp)
target_link_libraries(test_win_audio winmm)

add_executable(midi_win src/midi_in_sample.cpp)
target_link_libraries(midi_win winmm)