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

int32_t time_offset_ms = 0;
constexpr int32_t transmission_delay_ms = 0;
constexpr int32_t startup_delay_ms = 5000l;

void flashLed(unsigned int on_time);
int32_t millis_signed();

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
