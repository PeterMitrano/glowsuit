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
