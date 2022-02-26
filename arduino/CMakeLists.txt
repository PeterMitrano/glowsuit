function(suit suit_idx)
    add_executable(suit${suit_idx}
            suits/main.cpp
            suits/suit_program.cpp
            suits/suit_common.cpp
            suits/suit_${suit_idx}.cpp)
    target_link_arduino_libraries(suit${suit_idx} PRIVATE core XBee ArduinoSTL)
    target_include_directories(suit${suit_idx} PUBLIC suits ${CMAKE_CURRENT_SOURCE_DIR})
    target_enable_arduino_upload(suit${suit_idx})
endfunction()

suit(1)
suit(2)
suit(3)
suit(4)
suit(5)
