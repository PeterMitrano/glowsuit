#include <common.h>
#include <numeric>
#include <iostream>

std::pair<std::vector<uint8_t>, size_t> make_packet(State state)
{
    auto data = state.data;
    uint8_t const length_lb = message_size + 5;
    constexpr uint8_t api_identifier = 0x01;
    constexpr uint8_t frame_id = 0x01; // TODO: should we bother setting this?
    constexpr uint8_t address_lb = 0x01;
    constexpr uint8_t option = 0x00;

    // EXAMPLE: transmit channel 1 suit 1: 7E 00 0B 00 01 00 01 00 00 00 00 00 00 00 FC
    std::vector<uint8_t> packet{0x7E};
    packet.push_back(0x00);
    packet.push_back(length_lb);
    packet.push_back(api_identifier);
    packet.push_back(frame_id);
    packet.push_back(0x00);
    packet.push_back(address_lb);
    packet.push_back(option);
    for (auto const data_byte : data)
    {
        packet.push_back(data_byte);
    }
    // To calculate the checksum of an API frame:
    //   Add all bytes of the packet, except the start delimiter 0x7E and the length (the second and third bytes).
    //   Keep only the lowest 8 bits from the result. Subtract this quantity from 0xFF.
    constexpr auto mask = 0xFF;
    auto const checksum = 0xFF - (std::accumulate(packet.begin() + 3, packet.end(), 0u) & mask);
    packet.push_back(static_cast<uint8_t>(checksum));
    return {packet, packet.size()};
}

std::pair<std::vector<uint8_t>, size_t> make_packet(std::vector<uint8_t> const &data)
{
    uint8_t const length_lb = data.size() + 5;
    constexpr uint8_t api_identifier = 0x01;
    constexpr uint8_t frame_id = 0x01; // TODO: should we bother setting this?
    constexpr uint8_t address_lb = 0x01; // TODO: does this matter either?
    constexpr uint8_t option = 0x00;

    std::vector<uint8_t> packet{0x7E};
    packet.push_back(0x00); // length high bytes
    packet.push_back(length_lb);
    packet.push_back(api_identifier);
    packet.push_back(frame_id);
    packet.push_back(0x00); // address high bytes
    packet.push_back(address_lb);
    packet.push_back(option);
    for (auto const data_byte : data)
    {
        packet.push_back(data_byte);
    }
    // To calculate the checksum of an API frame:
    //   Add all bytes of the packet, except the start delimiter 0x7E and the length (the second and third bytes).
    //   Keep only the lowest 8 bits from the result. Subtract this quantity from 0xFF.
    constexpr auto mask = 0xFF;
    auto const checksum = 0xFF - (std::accumulate(packet.begin() + 3, packet.end(), 0u) & mask);
    packet.push_back(static_cast<uint8_t>(checksum));
    return {packet, packet.size()};
}

void print_packet(std::vector<uint8_t> const &packet)
{
    for (auto const byte : packet)
    {
        printf("%02X ", static_cast<unsigned int>(byte));
    }
    std::cout << std::endl;
}

Packet parse_packet(std::vector<uint8_t> const &data)
{
    Packet packet;

    auto const length_hb = data[1];
    auto const length_lb = data[2];
    auto const length = (length_hb << 8u) + length_lb;
    auto const data_length = length - 5;

    auto const source_address_hb = data[4];
    auto const source_address_lb = data[5];
    auto const source_address = (source_address_hb << 8u) + source_address_lb;

    packet.source_address = source_address;
    auto start_idx = 8u;
    for (auto offset = 0; offset < data_length; ++offset)
    {
        packet.data.push_back(data[start_idx + offset]);
    }

    return packet;
}
