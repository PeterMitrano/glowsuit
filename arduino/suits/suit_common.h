#pragma once

#include <QMetaType>
#include <vector>
#include <stdint.h>

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

constexpr int32_t transmission_delay_ms = 0;
constexpr int32_t startup_delay_ms = 5000l;

enum class CommandType : uint8_t
{
    Pause = 0x01,
    Time = 0x02,
    Resume = 0x03,
    State = 0x04,
    End
};

struct SuitCommand
{
    CommandType type = CommandType::End;
    std::vector<uint8_t> data;

    size_t size() const;

    explicit SuitCommand(CommandType type, std::vector<uint8_t> data) : type(type), data(data)
    {}

    explicit SuitCommand(CommandType type) : type(type)
    {}

    explicit SuitCommand()
    {}
};

Q_DECLARE_METATYPE(SuitCommand);

SuitCommand response_to_suit_command(uint8_t const *data, uint16_t length);

template<typename T>
T from_bytes(std::vector<uint8_t> bytes)
{
    // assumes big endian
    T value{0};
    for (uint16_t i = 0; i < bytes.size(); ++i)
    {
        T byte = bytes[i];
        value += byte << ((bytes.size() - i - 1) * 8);
    }

    return value;
}

void flashLed(unsigned int on_time);

void infinite_error(unsigned int on_time);
