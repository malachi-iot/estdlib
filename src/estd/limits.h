/**
 * @file
 * NOTE: My own exposure to embedded development has been 100% 8-bit oriented, so
 * these limits.h reflect that and focus on 8-bit-boundary computing
 */
#pragma once

#include "internal/platform.h"
#include "internal/fwd/limits.h"
#include "internal/limits.h"
#include "cstdint.h"
#include "cstddef.h"

// Reduced-dependency flavor - for integral_constant only
// DEBT: Might be better to make a new "traits/type/integral_constant.h" and friends to
// more clearly provide that
#include "internal/type_traits.h"

#if FEATURE_CPP_PUSH_MACRO
#include "internal/macro/push.h"
#endif

namespace estd {

// DEBT: Would likely be better off doing SIZEOF_INTEGER and friends with template
// metaprogramming to avoid potential name collisions
// size in 8-bit bytes
#define SIZEOF_INTEGER(max) \
    (max == INT64_MAX ? 8 : \
    (max == INT32_MAX ? 4 : \
    (max == INT16_MAX ? 2 : 1)))

#define SIZEOF_LLONG    SIZEOF_INTEGER(LLONG_MAX)
#define SIZEOF_LONG     SIZEOF_INTEGER(LONG_MAX)
#define SIZEOF_INT      SIZEOF_INTEGER(INT_MAX)
#define SIZEOF_SHORT    SIZEOF_INTEGER(SHRT_MAX)
#define SIZEOF_CHAR     SIZEOF_INTEGER(CHAR_MAX)

// DEBT: Move this portion out to 'internal' proper

namespace internal {

// DEBT: Strongly consider uint_strlen and integer_limits as candidates for 'detail' namespace
// they are useful on their own and are approaching stability.  Also, consider a consumer of that
// 'layer1::numeric_string'

template <unsigned base, unsigned bits>
struct uint_strlen;

// DEBT: I'm sure there's a more math-y way to do this

template <> struct uint_strlen<10, 8> :
        estd::integral_constant<unsigned, 3> {};

template <> struct uint_strlen<10, 16> :
        estd::integral_constant<unsigned, 5> {};

template <> struct uint_strlen<10, 32> :
        estd::integral_constant<unsigned, 10> {};

template <> struct uint_strlen<10, 64> :
        estd::integral_constant<unsigned, 20> {};

template <unsigned bits>
struct uint_strlen<16, bits> :
        estd::integral_constant<unsigned, bits / 4> {};

// DEBT: calculation approximate, higher the bitcount the more wasted space
template <unsigned bits>
struct uint_strlen<8, bits> :
        estd::integral_constant<unsigned, (bits + 1) / 3> {};

// Not the same as boost's version
template <class T, bool is_signed_>
struct signed_traits;

template <class T>
struct signed_traits<T, true>
{
    typedef T signed_type;
};

template <class T>
struct signed_traits<T, false>
{
    typedef T unsigned_type;
};


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

// NOTE: Did anyone ever hear of a non-signed floating point type?
template <class T, bool is_signed_ = true>
struct float_limits
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = false;
    static CONSTEXPR bool is_signed = is_signed_;

#if defined(__GCC_IEC_559) || defined(__STDC_IEC_559__)
    static CONSTEXPR bool is_iec559 = true;

#if FEATURE_ESTD_DRAGONBOX
    // NOTE: Shamelessly reworked from Dragonbox' max_output_string_length
    // DEBT: For 32 bit, is one byte too large due to exp len of 3 when it
    // should be 2
    template <class Format = jkj::dragonbox::default_float_traits<T> >
    struct length_exp :
        integral_constant<unsigned,
            // sign(1) + significand(9) + decimal_point + exp_marker + exp_sign + exp
            1 + Format::decimmal_digits + 1 + 1 + 3>::value >;
#endif

#else
    static CONSTEXPR bool is_iec559 = false;
#endif

};

// We maintain this internal one because it helps quite a bit with int -> precision
// mapping.
// DEBT: Just as above, strongly consider making this into 'detail' namespace
template <class T> struct numeric_limits;

template <>
struct numeric_limits<int8_t> : internal::integer_limits<int8_t, true>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR int8_t min() { return INT8_MIN; }
    static CONSTEXPR int8_t max() { return INT8_MAX; }

    typedef uint8_t unsigned_type;
};

template <>
struct numeric_limits<uint8_t> : internal::integer_limits<uint8_t, false>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR uint8_t min() { return 0; }
    static CONSTEXPR uint8_t max() { return UINT8_MAX; }

    typedef int8_t signed_type;
};

template <>
struct numeric_limits<int16_t> : internal::integer_limits<int16_t, true>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR int16_t min() { return INT16_MIN; }
    static CONSTEXPR int16_t max() { return INT16_MAX; }

    typedef uint16_t unsigned_type;
};

template <>
struct numeric_limits<uint16_t> : internal::integer_limits<uint16_t, false>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR uint16_t min() { return 0; }
    static CONSTEXPR uint16_t max() { return UINT16_MAX; }

    typedef int16_t signed_type;
};

template <>
struct numeric_limits<int32_t> : internal::integer_limits<int32_t, true>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR int32_t min() { return INT32_MIN; }
    static CONSTEXPR int32_t max() { return INT32_MAX; }

    typedef uint32_t unsigned_type;
};

template <>
struct numeric_limits<uint32_t> : internal::integer_limits<uint32_t, false>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR uint32_t min() { return 0; }
    static CONSTEXPR uint32_t max() { return UINT32_MAX; }

    typedef int32_t signed_type;
};

template <>
struct numeric_limits<int64_t> : internal::integer_limits<int64_t, true>
{
    static CONSTEXPR int digits10 = 18;
    static CONSTEXPR int64_t min() { return INT64_MIN; }
    static CONSTEXPR int64_t max() { return INT64_MAX; }

    typedef uint64_t unsigned_type;
};

template <>
struct numeric_limits<uint64_t> :  internal::integer_limits<uint64_t, false>
{
    static CONSTEXPR int digits10 = 19;
    static CONSTEXPR uint64_t min() { return 0; }
    static CONSTEXPR uint64_t max() { return UINT64_MAX; }

    typedef int64_t signed_type;
};

// NOTE: Just discovered the __SIZEOF_xxx__ paradigm, see if I can pare down my own version
// Basically experimental
#ifdef __SIZEOF_INT128__
template <>
struct numeric_limits<__int128> :  internal::integer_limits<__int128, true>
{
    static CONSTEXPR int digits10 = 19; // FIX: this is wrong, represents 64-bit size
    static CONSTEXPR uint64_t min() { return 0; }
    // FIX: the following is just an approximation for now
    static CONSTEXPR uint64_t max() { return UINT64_MAX * ((UINT64_MAX) / 2); }
};
#endif

}

#if SIZEOF_CHAR == 16
template <> struct numeric_limits<signed char> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned char> : internal::numeric_limits<uint16_t> {};
#else
template <> struct numeric_limits<signed char> : internal::numeric_limits<int8_t> {};
template <> struct numeric_limits<unsigned char> : internal::numeric_limits<uint8_t> {};
#endif

#if SIZEOF_SHORT == 4
template <> struct numeric_limits<short> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint32_t> {};
#else
template <> struct numeric_limits<short> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint16_t> {};
#endif


#if SIZEOF_INT == 8
template <> struct numeric_limits<int> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint64_t> {};
#elif SIZEOF_INT == 4
template <> struct numeric_limits<int> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint32_t> {};
#else
template <> struct numeric_limits<int> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint16_t> {};
#endif


#if SIZEOF_LONG == 8
template <> struct numeric_limits<long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint64_t> {};
#else
template <> struct numeric_limits<long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint32_t> {};
#endif

#if SIZEOF_LLONG == 8
template <> struct numeric_limits<long long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint64_t> {};
#else
template <> struct numeric_limits<long long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint32_t> {};
#endif

template <> struct numeric_limits<bool>
{
    static CONSTEXPR bool min() { return false; }
    static CONSTEXPR bool max() { return true; }

    static CONSTEXPR int digits = 1;
    static CONSTEXPR int digits10 = 0;
};

#ifdef __FLT_MIN__
template <>
struct numeric_limits<float> : internal::float_limits<float>
{
    static ESTD_CPP_CONSTEXPR_RET float lowest()
    {
        return -__FLT_MAX__;
    }

    static ESTD_CPP_CONSTEXPR_RET float min()
    {
        return __FLT_MIN__;
    }

    static ESTD_CPP_CONSTEXPR_RET float max()
    {
        return __FLT_MAX__;
    }
};
#endif

#ifdef __DBL_MIN__
template <>
struct numeric_limits<double> : internal::float_limits<double>
{
    static ESTD_CPP_CONSTEXPR_RET double lowest()
    {
        return -__DBL_MAX__;
    }

    static ESTD_CPP_CONSTEXPR_RET double min()
    {
        return __DBL_MIN__;
    }

    static ESTD_CPP_CONSTEXPR_RET double max()
    {
        return __DBL_MAX__;
    }
};
#endif

}

#if FEATURE_CPP_PUSH_MACRO
#include "internal/macro/pop.h"
#endif
