#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <utility>

#include <byteset.h>

constexpr int message_size = 6;

using State = Byteset<message_size>;

std::pair<std::vector<uint8_t>, size_t> make_packet(State state);

constexpr std::size_t num_suits = 6;
constexpr int midi_note_offset = 60;
constexpr int baud_rate = 57600;

constexpr int midi_note_off = 128;
constexpr int midi_note_on = 144;
