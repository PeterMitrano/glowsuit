#include <common.h>
#include <iomanip>
#include <iostream>
#include <numeric>

Data make_packet(State state)
{
    std::vector<uint8_t> data_vec(state.data.begin(), state.data.end());
    return make_packet(data_vec);
}

Data make_packet(SuitCommand const &command)
{
    std::vector<uint8_t> data_vec;
    data_vec.push_back(static_cast<uint8_t >(command.type));
    std::copy(command.data.cbegin(), command.data.cend(), std::back_inserter(data_vec));
    return make_packet(data_vec);
}

Data make_packet(std::vector<uint8_t> const &data)
{
    uint8_t const length_lb = data.size() + 5;
    constexpr uint8_t api_identifier = 0x01;
    constexpr uint8_t frame_id = 0x01; // TODO: should we bother setting this?
    constexpr uint8_t address_lb = 0x01; // TODO: does this matter either?
    constexpr uint8_t option = 0x00;

    Data packet{0x7E};
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
    return packet;
}

void print_packet(std::vector<uint8_t> const &packet)
{
    for (auto const byte : packet)
    {
        printf("%02X ", static_cast<unsigned int>(byte));
    }
    std::cout << std::endl;
};

std::optional<Packet> read_packet(serial::Serial *xbee_serial)
{
    Packet packet;
    std::vector<uint8_t> data;
    auto bytes_read = xbee_serial->read(data, 1);

    // check if a new packet is available
    if (bytes_read != 1 or data[0] != 0x7E)
    {
        return {};
    }

    // if it is, read the length and command ID
    bytes_read = xbee_serial->read(data, 3);

    if (bytes_read != 3)
    {
        // something went wrong -- abort!
        return {};
    }

    auto const length_hb = data[1];
    auto const length_lb = data[2];
    auto const length = (length_hb << 8u) + length_lb;

    auto const command_id = data[3];
    packet.command_id = command_id;

    bytes_read = xbee_serial->read(data, length);
    if (bytes_read != length)
    {
        // something went wrong -- abort!
        return {};
    }

    switch (command_id)
    {
        case RX_16: // RX (Receive) 16 bit
        {
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
        case TX_STATUS:
        {
            return packet;
        }
        default:
        {
            std::cout << "API command " << std::hex << command_id << " not supported!\n";
            return std::optional<Packet>();
        }
    }
}

std::optional<Packet> data_to_packet(Data const &data)
{
    if (data.empty())
    {
        return {};
    }
    // check if the start byte is right
    if (data[0] != 0x7E)
    {
        return {};
    }

    auto const length_hb = data[1];
    auto const length_lb = data[2];
    auto const length = (length_hb << 8u) + length_lb;

    auto const command_id = data[3];
    Packet packet;
    packet.command_id = command_id;

    if (length + 4 != data.size())
    {
        return {};
    }

    switch (command_id)
    {
        // this is weird, but since we're mocking XBee code sometimes, we sometimes need to parse a transmit message
        case TX_16:
        {
            auto const data_length = length - 5;
            auto const dest_address_hb = data[5];
            auto const dest_address_lb = data[6];
            auto const dest_address = (dest_address_hb << 8u) + dest_address_lb;
            packet.dest_address = dest_address;
            auto start_idx = 8u;
            for (auto offset = 0; offset < data_length; ++offset)
            {
                packet.data.push_back(data[start_idx + offset]);
            }
            return packet;
        }
        case RX_16: // RX (Receive) 16 bit
        {
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
        case TX_STATUS:
        {
            return packet;
        }
        default:
        {
            std::cout << "API command " << std::hex << command_id << " not supported!\n";
            return std::optional<Packet>();
        }
    }

}

Data suit_command_to_data(SuitCommand const command)
{
    Data data{};

    auto const command_type = static_cast<uint8_t>(command.type);
    data.push_back(command_type);
    for (auto i{0u}; i < command.size(); ++i)
    {
        data.push_back(command.data[i]);
    }
    return data;
}

std::ostream &operator<<(std::ostream &os, const SuitCommand &command)
{
    os << "[";
    switch (command.type)
    {
        case CommandType::Time:
            os << "TIME   ";
            break;
        case CommandType::Pause:
            os << "PAUSE  ";
            break;
        case CommandType::Resume:
            os << "RESUME ";
            break;
        case CommandType::State:
            os << "STATE  ";
            break;
        case CommandType::End:
            break;
    }
    std::ios::fmtflags old_settings = std::cout.flags();
    for (auto i{0u}; i < command.size(); ++i)
    {
        os << std::hex
           << std::showbase // show the 0x prefix
           << std::internal // fill between the prefix and the number
           << std::setfill('0') // fill with 0s
           << static_cast<int>(command.data[i])
           << " ";
    }
    os << "]";
    std::cout.flags(old_settings);
    return os;
}

std::ostream &operator<<(std::ostream &os, const Data &data)
{
    os << "[";
    std::ios::fmtflags old_settings = std::cout.flags();
    for (auto i{0u}; i < data.size(); ++i)
    {
        os << std::hex
           << std::showbase // show the 0x prefix
           << std::internal // fill between the prefix and the number
           << std::setfill('0') // fill with 0s
           << static_cast<int>(data[i])
           << " ";
    }
    os << "]";
    std::cout.flags(old_settings);
    return os;
}
