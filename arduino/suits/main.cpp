#include <Arduino.h>
#include <XBee.h>

#include "suit_program.h"
#include "suit_choreo.h"

XBee xbee;
SuitProgram suit_program(choreo, num_events);

void setup()
{
    suit_program.setup();
}


void loop()
{
    suit_program.loop();
}
