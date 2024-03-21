#pragma once

// See:
// https://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html
// https://onlinedocs.microchip.com/pr/GUID-317042D4-BCCE-4065-BB05-AC4312DBC2C4-en-US-2/index.html?GUID-CF4CBBD9-1656-4D92-8EC2-853B1AEDF6F6  
// Also interesting:
// https://android.googlesource.com/toolchain/avr-libc/+/9b9d708c6a8a66a75d9fdf8eb6b35305bb1712e3/avr-libc-1.7.1/include/avr/pgmspace.h

#include <avr/pgmspace.h>

namespace estd { namespace internal {

// DEBT: Due to lack of partial specialization on functions, this default
// flavor only works for near addresses
template <typename T, bool near = true>
inline T pgm_read(const void* address)
{
    T temp;

    // Presumes a "blessed" function.  We might want to manualy do a bunch of
    // lpms for small-ish types.
    memcpy_P(&temp, address, sizeof(T));

    return temp;
}

template <>
inline char pgm_read<char, true>(const void* address)
{
    return pgm_read_byte_near(address);
}

template <>
inline uint8_t pgm_read<uint8_t, true>(const void* address)
{
    return pgm_read_byte_near(address);
}


#ifdef __AVR_HAVE_ELPM__
template <>
inline char pgm_read<char, false>(const void* address)
{
    return pgm_read_byte_far(address);
}
#endif

template <>
inline uint16_t pgm_read<uint16_t>(const void* address)
{
    return pgm_read_word_near(address);
}

template <>
inline int16_t pgm_read<int16_t>(const void* address)
{
    return pgm_read_word_near(address);
}


template <>
inline uint32_t pgm_read<uint32_t>(const void* address)
{
    return pgm_read_dword_near(address);
}


template <>
inline int32_t pgm_read<int32_t>(const void* address)
{
    return pgm_read_dword_near(address);
}


}}