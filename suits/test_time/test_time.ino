#include <XBee.h>

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

int32_t millis_signed()
{
    return static_cast<int32_t>(millis());
}

int32_t time_offset_ms = 0;
constexpr int32_t transmission_delay_ms = 25;
constexpr int32_t startup_delay = 5000l;

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
    pinMode(led_pin, OUTPUT);
    for (auto const pin : channel_to_pin)
    {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(57600);
    xbee.setSerial(Serial);

    digitalWrite(led_pin, HIGH);
    xbee.readPacketUntilAvailable();
    update_time();

    digitalWrite(led_pin, LOW);

    // now wait until the global start time (N seconds) is reached
    while ((millis_signed() - time_offset_ms) <= startup_delay)
    {
        // read packets so they dont just size in our buffer
        xbee.readPacket();
    }
}

uint32_t idx = 1;

void loop()
{
    int32_t const now = millis_signed() - time_offset_ms;
    int32_t const choreo_time = now - startup_delay;
    int32_t const next_onset_ms = idx * 1000ul;

    if (choreo_time >= next_onset_ms)
    {
        ++idx;
        digitalWrite(led_pin, HIGH);
        delay(40);
        digitalWrite(led_pin, LOW);
    }

    xbee.readPacket();
    update_time();
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
