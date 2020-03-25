#include <Arduino.h>
#include <XBee.h>

#include "scoped_suit_main.h"
#include "suit_choreo.h"

int32_t time_offset_ms;
XBee xbee;

void setup()
{
    setupScoped(xbee, time_offset_ms, suit_number);
}


void loop()
{
    loopScoped(xbee, time_offset_ms, num_events, choreo);
}
