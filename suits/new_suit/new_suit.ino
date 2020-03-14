#include <XBee.h>
#include <SoftwareSerial.h>

#include "suit_3.h"

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
Rx16Response rx16 = Rx16Response();

constexpr int led_pin = 13;

constexpr int channel_to_pin[8] = {
        2,  // 0
        3,   // 1
        4,   // 2
        5,   // 3
        6,  // 4
        7,  // 5
        8,  // 6
        9,  // 7
};

void flashLed(unsigned int on_time);

int32_t time_offset_ms = 0;
constexpr int32_t transmission_delay_ms = 50;
constexpr int32_t startup_delay_ms = 5000l;

int32_t millis_signed()
{
    return static_cast<int32_t>(millis());
}

template<typename T>
T from_bytes(uint8_t *bytes, uint8_t length)
{
    // assumes big endian
    T value{0};
    for (int32_t i = 0; i < length; ++i)
    {
        T byte = bytes[i];
        value += byte << ((length - i - 1) * 8);
    }

    return value;
}

void setup()
{
    delay(1000);

    pinMode(led_pin, OUTPUT);
    for (auto const pin : channel_to_pin)
    {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(57600);

    for (auto i{0u}; i < suit_number; ++i)
    {
        flashLed(200);
    }

    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, HIGH);
    }

    xbee.setSerial(Serial);
    xbee.readPacketUntilAvailable();
    update_time();

    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, LOW);
    }

    // now wait until the global start time (N seconds) is reached
    while ((millis_signed() - time_offset_ms) <= startup_delay_ms)
    {
        // read packets so they dont just size in our buffer
        xbee.readPacket();
    }
}

uint16_t choreo_idx = 0;

void loop()
{
    if (choreo_idx >= num_events - 1)
    {
        for (auto const pin : channel_to_pin)
        {
            digitalWrite(pin, LOW);
        }
        return;
    };

    int32_t const now_ms = millis_signed() - time_offset_ms;
    int32_t const choreo_time_cs = (now_ms - startup_delay_ms) / 10;
    auto const next_onset_cs = pgm_read_word_near(choreo + (choreo_idx * 3));

    if (choreo_time_cs >= next_onset_cs)
    {
        // display the current state
        auto const current_state = pgm_read_byte_near(choreo + (choreo_idx * 3 + 2));
        size_t bit_idx = 0;
        for (auto const pin : channel_to_pin)
        {
            auto const channel_on = (bool) ((current_state >> bit_idx) & 0x01);
            digitalWrite(pin, channel_on ? HIGH : LOW);
            ++bit_idx;
        }

        // move on to next event
        ++choreo_idx;
    }


    xbee.readPacket();
    update_time();
}

void flashLed(unsigned int on_time)
{
    // just for debugging
    digitalWrite(led_pin, HIGH);
    delay(on_time);
    digitalWrite(led_pin, LOW);
    delay(200);
}

void update_time()
{
    if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == RX_16_RESPONSE)
    {
        xbee.getResponse().getRx16Response(rx16);
        auto const current_global_time_ms = from_bytes<int32_t>(rx16.getData(), rx16.getDataLength());
        time_offset_ms = millis_signed() - current_global_time_ms + transmission_delay_ms;
    }
}
