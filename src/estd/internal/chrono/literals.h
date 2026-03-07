#pragma once

#include "duration.h"

// Just a formality, they usually are present together
#if __cpp_user_defined_literals && defined(FEATURE_CPP_INLINE_NAMESPACE)
namespace estd { inline namespace literals { inline namespace chrono_literals {

// CLang allows this during the compile phase, but ignores the definitions and generates
// a bunch of warnings.  Until we can repair that, CLang is disabled.  In the short term,
// we only enable these literals for GCC
#if defined(__GNUC__) && !defined(__clang__) && __cpp_user_defined_literals
#define FEATURE_ESTD_CHRONO_LITERALS 1

// Compiler complains that these are in the reserved suffix category, which is in a sense
// true - i.e. an actual std::literal::chrono_literals ""s is "correct", but we like to use
// our own version of seconds, microseconds, etc.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"

constexpr chrono::seconds operator ""s(unsigned long long s)
{
    return chrono::seconds(s);
}

constexpr chrono::milliseconds operator ""ms(unsigned long long ms)
{
    return chrono::milliseconds(ms);
}

constexpr chrono::microseconds operator ""us(unsigned long long rep)
{
    return chrono::microseconds(rep);
}

constexpr chrono::nanoseconds operator ""ns(unsigned long long rep)
{
    return chrono::nanoseconds(rep);
}

#pragma GCC diagnostic pop
#else
#define FEATURE_ESTD_CHRONO_LITERALS 0
#endif

}}}
#endif

