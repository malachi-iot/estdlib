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
ESTD_CPP_CONSTEXPR(14) Unsigned bit_packed_read_be_lsb(const uint8_t* data)
{
    constexpr unsigned byte_pos = pos / 8;
    constexpr unsigned mask = (1U << width) - 1;
    constexpr unsigned bit_pos = pos % 8;

    static_assert(bit_pos + width <= 8, "Spanning bytes not yet supported");

    Unsigned v;

    data += byte_pos;

    v = *data;
    v >>= bit_pos;
    v &= mask;

    return v;
}

template <unsigned pos, unsigned width, class Int>
ESTD_CPP_CONSTEXPR(14) void bit_packed_write_be_lsb(uint8_t* data, Int value)
{
    constexpr unsigned byte_pos = pos / 8;
    constexpr unsigned mask = (1U << width) - 1;
    constexpr unsigned bit_pos = pos % 8;

    static_assert(bit_pos + width <= 8, "Spanning bytes not yet supported");

    data += byte_pos;

    value <<= bit_pos;

    *data &= ~(mask << bit_pos);
    *data |= value;
}

/// Set all bits to zero
template <unsigned pos, unsigned width, class Int>
ESTD_CPP_CONSTEXPR(14) void bit_packed_reset_lsb(uint8_t* data)
{
    static_assert(pos == 0, "Only byte-aligned support at this time");
    static_assert(width % 8 == 0, "Only byte aligned supported at this time");

    //constexpr unsigned byte_pos = pos / 8;
    //constexpr unsigned mask = (1U << width) - 1;
    //constexpr unsigned bit_pos = pos % 8;

    // UNTESTED
    for(unsigned byte_width = width % 8; byte_width != 0; --byte_width, --data)
    {
        *data = 0;
    }
}

template <unsigned pos, unsigned width, class Int = unsigned>
struct bit_packed
{
    static constexpr Int read(const uint8_t* data)
    {
        return bit_packed_read_be_lsb<pos, width, Int>(data);
    }

    static ESTD_CPP_CONSTEXPR(14) void write(uint8_t* data, Int value)
    {
        bit_packed_write_be_lsb<pos, width>(data, value);
    }

    // UNTESTED
    static ESTD_CPP_CONSTEXPR(14) void reset(uint8_t* data)
    {
        bit_packed_reset_lsb<pos, width>(data);
    }
};

}}