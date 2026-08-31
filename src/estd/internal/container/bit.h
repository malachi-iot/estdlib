#pragma once

#include "../../cstdint.h"

namespace estd { namespace internal {

// DEBT: Not a substitute for embr bit library, but we need the basics here
// for alignment-predictable bit packed struct like behavior (from unordered_map)
// As such, these operations always:
// - treat pos as lsb -> msb, so conceptually rightmost bit as lsb first, then moves leftward
// NOTE: values must not only be smaller than 1 byte but also at this time must not span byte boundaries
// Eventually:
// - treat integers as big endian (network order)

template <unsigned pos, unsigned width, class Unsigned = unsigned>
ESTD_CPP_CONSTEXPR(14) Unsigned bit_packed_read(const uint8_t* data)
{
    constexpr unsigned byte_pos = pos / 8;
    constexpr unsigned mask = (1 << width) - 1;
    constexpr unsigned bit_pos = pos % 8;

    Unsigned v;

    data += byte_pos;

    v = *data;
    v >>= bit_pos;
    v &= mask;

    return v;
}

template <unsigned pos, unsigned width, class Int>
ESTD_CPP_CONSTEXPR(14) void bit_packed_write(uint8_t* data, Int value)
{
    constexpr unsigned byte_pos = pos / 8;
    constexpr unsigned mask = (1 << width) - 1;
    constexpr unsigned bit_pos = pos % 8;

    data += byte_pos;

    value <<= bit_pos;

    *data &= ~mask;
    *data |= value;
}

template <unsigned pos, unsigned width, class Int = unsigned>
struct bit_packed
{
    static constexpr Int read(const uint8_t* data)
    {
        return bit_packed_read<pos, width, Int>(data);
    }

    static ESTD_CPP_CONSTEXPR(14) void write(uint8_t* data, Int value)
    {
        bit_packed_write<pos, width>(data, value);
    }
};

}}