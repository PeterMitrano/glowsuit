#pragma once

#include <sstream>
#include <bitset>
#include <cstdint>
#include <array>

template<unsigned int size>
class Byteset
{
public:
    std::array<uint8_t, size> data;

    explicit Byteset()
    {
        data.fill(0);
    }

    void set(size_t overall_bit_index, bool value)
    {
        auto const byte_idx = static_cast<size_t>(overall_bit_index / 8);
        auto const bit_idx = overall_bit_index % 8;
        if (byte_idx >= size)
        {
            std::stringstream ss;
            ss << "size " << size << " byte idx " << byte_idx;
            throw std::out_of_range(ss.str());
        }
        auto const mask = 1 << bit_idx;
        // http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
        data[byte_idx] ^= ((-static_cast<int>(value)) ^ data[byte_idx]) & mask;
    }

};

template<unsigned int size>
inline
std::ostream &operator<<(std::ostream &os, const Byteset<size> &byteset)
{
    for (auto const &byte : byteset.data)
    {
        for (auto i = 0u; i < 8; ++i)
        {
            auto const c = byte & (1 << i) ? '1' : '0';
            os << c;
        }
        os << " ";
    }
    return os;
}
