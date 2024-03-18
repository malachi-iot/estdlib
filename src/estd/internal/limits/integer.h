#pragma once

#include "traits.h"
#include "../../cstdint.h"

namespace estd { namespace internal {

// DEBT: Strongly consider uint_strlen and integer_limits as candidates for 'detail' namespace
// they are useful on their own and are approaching stability.  Also, consider a consumer of that
// 'layer1::numeric_string'


template <class T, bool is_signed_>
struct integer_limits : signed_traits<T, is_signed_>
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = true;
    static CONSTEXPR bool is_signed = is_signed_;
    static CONSTEXPR int digits = bits - (is_signed ? 1 : 0);
    static CONSTEXPR bool is_iec559 = false;
    static CONSTEXPR unsigned radix = 2;

    /// Retrieves maximum length a string of this int, accounting for
    /// base representation
    /// NOTE: this is an estd extension, not part of std
    /// DEBT: Only base 10 minus sign SHOULD be considered, but char conversion code doesn't
    /// filter out the notion of negative hex, for example.  That really does need to be sorted out,
    /// but in the meantime we overallocate here to avoid a stack corruption
    /// \tparam base supported values: 8, 10, 16
    template <unsigned base>
    struct length :
        integral_constant<unsigned, uint_strlen<base, bits>::value +
                                    //((is_signed && base == 10) ? 1 : 0)> {};
                                    (is_signed ? 1 : 0)> {};
};

template <>
struct numeric_limits<int8_t> : integer_limits<int8_t, true>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR int8_t min() { return INT8_MIN; }
    static CONSTEXPR int8_t max() { return INT8_MAX; }

    typedef uint8_t unsigned_type;
};

template <>
struct numeric_limits<uint8_t> : integer_limits<uint8_t, false>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR uint8_t min() { return 0; }
    static CONSTEXPR uint8_t max() { return UINT8_MAX; }

    typedef int8_t signed_type;
};

template <>
struct numeric_limits<int16_t> : integer_limits<int16_t, true>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR int16_t min() { return INT16_MIN; }
    static CONSTEXPR int16_t max() { return INT16_MAX; }

    typedef uint16_t unsigned_type;
};

template <>
struct numeric_limits<uint16_t> : integer_limits<uint16_t, false>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR uint16_t min() { return 0; }
    static CONSTEXPR uint16_t max() { return UINT16_MAX; }

    typedef int16_t signed_type;
};

template <>
struct numeric_limits<int32_t> : integer_limits<int32_t, true>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR int32_t min() { return INT32_MIN; }
    static CONSTEXPR int32_t max() { return INT32_MAX; }

    typedef uint32_t unsigned_type;
};

template <>
struct numeric_limits<uint32_t> : integer_limits<uint32_t, false>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR uint32_t min() { return 0; }
    static CONSTEXPR uint32_t max() { return UINT32_MAX; }

    typedef int32_t signed_type;
};

template <>
struct numeric_limits<int64_t> : integer_limits<int64_t, true>
{
    static CONSTEXPR int digits10 = 18;
    static CONSTEXPR int64_t min() { return INT64_MIN; }
    static CONSTEXPR int64_t max() { return INT64_MAX; }

    typedef uint64_t unsigned_type;
};

template <>
struct numeric_limits<uint64_t> :  integer_limits<uint64_t, false>
{
    static CONSTEXPR int digits10 = 19;
    static CONSTEXPR uint64_t min() { return 0; }
    static CONSTEXPR uint64_t max() { return UINT64_MAX; }

    typedef int64_t signed_type;
};


#ifdef __SIZEOF_INT128__
template <>
struct numeric_limits<__int128> :  integer_limits<__int128, true>
{
    static CONSTEXPR int digits10 = 19; // FIX: this is wrong, represents 64-bit size
    static CONSTEXPR uint64_t min() { return 0; }
    // FIX: the following is just an approximation for now
    static CONSTEXPR __int128 max() { return UINT64_MAX * ((UINT64_MAX) / 2); }
};
#endif


}}
