#pragma once

#include <byteset.h>
#include <serial/serial.h>
#include <suit_common.h>

#include <QMetaType>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

constexpr int BytesPerMessage = 6;

using State = Byteset<BytesPerMessage>;
using Data = std::vector<uint8_t>;

Data suit_command_to_data(SuitCommand const command);

// FIXME: refactor the software-suit-xbee stuff out into separate library?
Data make_packet(State state);

Data make_packet(SuitCommand const &command);

Data make_packet(std::vector<uint8_t> const &data);

constexpr uint8_t TX_16{0x01};
constexpr uint8_t RX_16{0x81};
constexpr uint8_t TX_STATUS{0x89};

struct Packet {
  uint16_t source_address{0};
  uint16_t dest_address{0};
  uint8_t command_id{0};
  std::vector<uint8_t> data;
};

std::optional<Packet> data_to_packet(std::vector<uint8_t> const &data);

std::optional<Packet> read_packet(serial::Serial *xbee_serial);

void print_packet(std::vector<uint8_t> const &packet);

constexpr std::size_t num_suits = 5;
constexpr int midi_note_offset = 60;
constexpr int baud_rate = 57600;

constexpr int midi_note_off = 128;
constexpr int midi_note_on = 144;

template <typename T>
std::vector<uint8_t> to_bytes(T t) {
  std::vector<uint8_t> bytes;
  for (auto i{0u}; i < sizeof(t); ++i) {
    uint8_t const byte = (t >> (i * 8)) & 0xFF;
    bytes.emplace_back(byte);
  }
  std::reverse(bytes.begin(), bytes.end());
  return bytes;
}

Q_DECLARE_METATYPE(Data)

std::ostream &operator<<(std::ostream &os, const SuitCommand &command);

std::ostream &operator<<(std::ostream &os, const Data &data);
