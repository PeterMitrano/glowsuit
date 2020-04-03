#pragma once

#include <Arduino.h>
#include <XBee.h>

#include "suit_common.h"

int32_t millis_signed();

class SuitProgram
{
public:
    explicit SuitProgram(uint8_t const *choreo, uint16_t num_events);

    void setup();

    void loop();

    void handle_packets();

    void update_time(SuitCommand const &cmd);

    void all_on();

    void all_off();

private:
    bool paused{false};
    XBee xbee;
    int32_t time_offset_ms{0};
    uint8_t suit_number{0};
    uint8_t const *choreo;
    uint16_t const num_events;

};
