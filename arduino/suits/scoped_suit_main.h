#include <Arduino.h>
#include <XBee.h>

#include "suit_common.h"

int32_t millis_signed();

void setupScoped(XBee &scoped_xbee,
                 int32_t scoped_time_offset_ms,
                 uint8_t scoped_suit_number);

void loopScoped(XBee &scoped_xbee,
                int32_t scoped_time_offset_ms,
                uint8_t scoped_num_events,
                uint8_t const *scoped_choreo);

void flashLed(unsigned int on_time);

int32_t update_time(XBee &scoped_xbee, int32_t scoped_time_offset_ms);
