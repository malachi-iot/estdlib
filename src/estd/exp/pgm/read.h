#pragma once

#include <avr/pgmspace.h>

namespace estd { namespace internal {

template <typename T, bool near = true>
T pgm_read(const void* address);

template <>
inline char pgm_read<char, true>(const void* address)
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
inline uint32_t pgm_read<uint32_t>(const void* address)
{
    return pgm_read_dword_near(address);
}


}}