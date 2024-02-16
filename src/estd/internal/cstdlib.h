#pragma once

#if FEATURE_STD_CSTDLIB
#include <cstdlib>
#else
#include <stdlib.h>
#endif

namespace estd { namespace internal {

// NOTE: Not making these just functions because I like not using decltype to get 'result' type,
// plus it's a little more c++03 friendly

template <class Int>
struct div
{
    struct result
    {
        const Int quot;
        const Int rem;
    };

    static ESTD_CPP_CONSTEXPR_RET result eval(Int x, Int y)
    {
        return result { static_cast<Int>(x / y), static_cast<Int>(x % y) };
    }
};

template <>
struct div<int>
{
#if FEATURE_STD_CSTDLIB
    using result = std::div_t;

    static result eval(int x, int y) { return std::div(x, y); }
#else
    using result = ::div_t;

    static result eval(int x, int y) { return ::div(x, y); }
#endif
};


template <>
struct div<long>
{
#if FEATURE_STD_CSTDLIB
    using result = std::ldiv_t;

    static result eval(long x, long y) { return std::div(x, y); }
#else
    using result = ::ldiv_t;

    static result eval(int x, int y) { return ::ldiv(x, y); }
#endif
};


}}