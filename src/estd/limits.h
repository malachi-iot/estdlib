#pragma once

#include "cstddef.h"
#include "cstdint.h"
#include "internal/feature/estd.h"
#include "internal/limits.h"

#include "internal/macro/push.h"

namespace estd {

// sizeof(char) "is always 1" https://isocpp.org/wiki/faq/intrinsic-types#sizeof-char
template <> struct numeric_limits<signed char> : internal::numeric_limits<int8_t> {};
template <> struct numeric_limits<unsigned char> : internal::numeric_limits<uint8_t> {};

#if SHORT_WIDTH == 32
template <> struct numeric_limits<short> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint32_t> {};
#else
template <> struct numeric_limits<short> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint16_t> {};
#endif


#if INT_WIDTH == 64
template <> struct numeric_limits<int> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint64_t> {};
#elif INT_WIDTH == 32
template <> struct numeric_limits<int> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint32_t> {};
#elif INT_WIDTH == 16
template <> struct numeric_limits<int> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint16_t> {};
#else
#warning Unexpected integer size, not specialized
#endif


#if LONG_WIDTH == 64
template <> struct numeric_limits<long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint64_t> {};
#elif LONG_WIDTH == 32
template <> struct numeric_limits<long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint32_t> {};
#else
#error Failed SIZEOF_LONG sanity check
#endif

#if LLONG_WIDTH == 64
template <> struct numeric_limits<long long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint64_t> {};
#elif LLONG_WIDTH == 32
// DEBT: I am not convinced any has a 32-bit long long, and I am not convinced anyone should
template <> struct numeric_limits<long long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint32_t> {};
#elif FEATURE_ESTD_COMPILE_VERBOSITY > 2
#warning Failed SIZEOF_LLONG sanity check
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

    static CONSTEXPR unsigned digits = FLT_MANT_DIG;
    static CONSTEXPR unsigned digits10 = FLT_DIG;
#if FLT_DECIMAL_DIG
    static CONSTEXPR unsigned max_digita10 = FLT_DECIMAL_DIG;
#endif
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

    static CONSTEXPR unsigned digits = DBL_MANT_DIG;
    static CONSTEXPR unsigned digits10 = DBL_DIG;
#if DBL_DECIMAL_DIG
    static CONSTEXPR unsigned max_digita10 = DBL_DECIMAL_DIG;
#endif
};
#endif

}

#include "internal/macro/pop.h"

