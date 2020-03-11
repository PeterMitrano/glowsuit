#pragma once

#include <optional>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>

#include <byteset.h>
#include <serial/serial.h>

constexpr int message_size = 6;

using State = Byteset<message_size>;

// FIXME: refactor xbee out into separate library
std::pair<std::vector<uint8_t>, size_t> make_packet(State state);

std::pair<std::vector<uint8_t>, size_t> make_packet(std::vector<uint8_t> const &data);

constexpr uint8_t RX_16{0x81};
constexpr uint8_t TX_STATUS{0x89};

struct Packet
{
    uint16_t source_address{0};
    uint8_t command_id{0};
    std::vector<uint8_t> data;
};

std::optional<Packet> read_packet(serial::Serial *xbee_serial);

void print_packet(std::vector<uint8_t> const &packet);

constexpr std::size_t num_suits = 6;
constexpr int midi_note_offset = 60;
constexpr int baud_rate = 57600;

constexpr int midi_note_off = 128;
constexpr int midi_note_on = 144;

template<typename T>
std::vector<uint8_t> to_bytes(T t)
{
    std::vector<uint8_t> bytes;
    for (auto i{0u}; i < sizeof(t); ++i)
    {
        uint8_t const byte = (t >> (i * 8)) & 0xFF;
        bytes.emplace_back(byte);
    }
    std::reverse(bytes.begin(), bytes.end());
    return bytes;
}
