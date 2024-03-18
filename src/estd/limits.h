#pragma once

#include "cstddef.h"
#include "cstdint.h"
#include "internal/limits.h"

#include "internal/macro/push.h"

namespace estd {

// sizeof(char) "is always 1" https://isocpp.org/wiki/faq/intrinsic-types#sizeof-char
template <> struct numeric_limits<signed char> : internal::int_limits<CHAR_WIDTH> {};
template <> struct numeric_limits<unsigned char> : internal::uint_limits<CHAR_WIDTH> {};

template <> struct numeric_limits<short> : internal::int_limits<SHRT_WIDTH> {};
template <> struct numeric_limits<unsigned short> : internal::uint_limits<SHRT_WIDTH> {};

template <> struct numeric_limits<int> : internal::int_limits<INT_WIDTH> {};
template <> struct numeric_limits<unsigned int> : internal::uint_limits<INT_WIDTH> {};

template <> struct numeric_limits<long> : internal::int_limits<LONG_WIDTH> {};
template <> struct numeric_limits<unsigned long> : internal::uint_limits<LONG_WIDTH> {};

template <> struct numeric_limits<long long> : internal::int_limits<LLONG_WIDTH> {};
template <> struct numeric_limits<unsigned long long> : internal::uint_limits<LLONG_WIDTH> {};

template <> struct numeric_limits<bool>
{
    static ESTD_CPP_CONSTEXPR_RET bool min() { return false; }
    static ESTD_CPP_CONSTEXPR_RET bool max() { return true; }

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

