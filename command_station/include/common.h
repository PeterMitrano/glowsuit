#pragma once

#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <utility>

#include <byteset.h>

constexpr int BytesPerMessage = 6;

using State = Byteset<BytesPerMessage>;

std::pair<std::vector<uint8_t>, size_t> make_packet(State state);

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
