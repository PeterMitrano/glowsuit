#include <Arduino.h>
#include <XBee.h>

#include "suit_common.h"
#include "scoped_suit_main.h"

int32_t millis_signed()
{
    return static_cast<int32_t>(millis());
}

void setupScoped(XBee &scoped_xbee,
                 int32_t scoped_time_offset_ms,
                 uint8_t const scoped_suit_number)
{
    delay(1000);

    pinMode(led_pin, OUTPUT);
    for (auto const pin : channel_to_pin)
    {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(57600);

    for (auto i{0u}; i < scoped_suit_number; ++i)
    {
        flashLed(200);
    }

    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, HIGH);
    }

    scoped_xbee.setSerial(Serial);
    scoped_xbee.readPacketUntilAvailable();

    scoped_time_offset_ms = update_time(scoped_xbee, scoped_time_offset_ms);

    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, LOW);
    }

    // now wait until the global start time (N seconds) is reached
    while ((millis_signed() - scoped_time_offset_ms) <= startup_delay_ms)
    {
        // read packets so they dont just size in our buffer
        scoped_xbee.readPacket();
    }
}

void loopScoped(XBee &scoped_xbee,
                int32_t scoped_time_offset_ms,
                uint8_t const scoped_num_events,
                uint8_t const *scoped_choreo)
{
    int32_t const now_ms = millis_signed() - scoped_time_offset_ms;
    int32_t const choreo_time_cs = (now_ms - startup_delay_ms) / 10;

    // Find the next upcoming event and set choreo_idx
    uint16_t choreo_idx = scoped_num_events;
    for (auto j = 0u; j < scoped_num_events; ++j)
    {
        auto const onset_cs = pgm_read_word_near(scoped_choreo + (j * 3));
        if (onset_cs >= choreo_time_cs)
        {
            choreo_idx = j;
            break;
        }
    }

    if (choreo_idx >= scoped_num_events)
    {
        for (auto const pin : channel_to_pin)
        {
            digitalWrite(pin, LOW);
        }
        return;
    };

    auto const next_onset_cs = pgm_read_word_near(scoped_choreo + (choreo_idx * 3));
    if (choreo_time_cs >= next_onset_cs)
    {
        // display the current state
        auto const current_state = pgm_read_byte_near(scoped_choreo + (choreo_idx * 3 + 2));
        size_t bit_idx = 0;
        for (auto const pin : channel_to_pin)
        {
            auto const channel_on = (bool) ((current_state >> bit_idx) & 0x01);
            digitalWrite(pin, channel_on ? HIGH : LOW);
            ++bit_idx;
        }
    }


    scoped_xbee.readPacket();
    scoped_time_offset_ms = update_time(scoped_xbee, scoped_time_offset_ms);
}

void flashLed(unsigned int on_time)
{
    // just for debugging
    digitalWrite(led_pin, HIGH);
    delay(on_time);
    digitalWrite(led_pin, LOW);
    delay(200);
}

int32_t update_time(XBee &scoped_xbee, int32_t scoped_time_offset_ms)
{
    auto const available = scoped_xbee.getResponse().isAvailable();
    auto const api_id = scoped_xbee.getResponse().getApiId();
    if (available and api_id == RX_16_RESPONSE)
    {
        Rx16Response rx16 = Rx16Response();
        scoped_xbee.getResponse().getRx16Response(rx16);
        auto const current_global_time_ms = from_bytes<int32_t>(rx16.getData(), rx16.getDataLength());
        scoped_time_offset_ms = millis_signed() - current_global_time_ms - transmission_delay_ms;
    }
    return scoped_time_offset_ms;
}
