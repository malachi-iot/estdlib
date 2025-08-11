#pragma once

#include "internal/type_traits/is_trivial.h"

#include "internal/fwd/flags.h"

namespace estd { namespace detail { inline namespace v1 {

///
/// Designed really to be fully consteval friendly, but works as intended merely as constexpr
///
template <class Enum>
class flags
{
public:
#if FEATURE_ESTD_UNDERLYING_TYPE
    using int_type = typename estd::underlying_type<Enum>::type;
#else
    using int_type = int;
#endif

    using value_type = Enum;

private:
    value_type value_;

    // Force int conversion to always fail, we don't want 'bool' conversion to fool us in these cases
    //constexpr operator int_type() const { return value_; }

public:
    constexpr explicit flags(int_type v) :
        value_{value_type(v)}
    {}

    constexpr flags(const value_type& value) : value_{value}    {}

    ESTD_CPP_CONSTEXPR(14) flags& operator|=(flags v)
    {
        value_ |= v;
        return *this;
    }

    constexpr operator value_type() const { return value_; }

#if __GNUC_PREREQ(7,3)
    constexpr operator bool() const { return value_ != value_type{}; }
#endif

    constexpr bool is_set() const { return value_ != value_type{}; }

    // EXPERIMENTAL - works, but clumsy
    //constexpr bool operator ()() const { return value_ != value_type{}; }

    constexpr value_type value() const { return value_; }

    // Putting these all as members instead of freestanding operators for easy access to int_type

    constexpr flags operator~() const
    {
        return flags(~int_type(value_));
    }

    constexpr flags operator ^(value_type v) const
    {
        return flags{int_type(v) ^ int_type(value_)};
    }

    constexpr flags operator |(value_type v) const
    {
        return flags{int_type(v) | int_type(value_)};
    }

    constexpr flags operator &(value_type v) const
    {
        return flags{int_type(v) & int_type(value_)};
    }
};


template <class Enum>
constexpr bool operator==(const flags<Enum>& lhs, const Enum& rhs)
{
    return lhs.value() == rhs;
}


template <class Enum>
constexpr bool operator==(const flags<Enum>& lhs, const flags<Enum>& rhs)
{
    return lhs.value() == rhs.value();
}

template <class Enum>
constexpr bool operator!=(const flags<Enum>& lhs, const flags<Enum>& rhs)
{
    return lhs.value() != rhs.value();
}


}}}

namespace estd {

//template <typename Enum, Enum e, class T = void>
//struct if_flagged;

//template <typename Enum, Enum e, class T>
//struct if_flagged<detail::flags<Enum>, e, T> : enable_if<bool(e), T>  {};

namespace detail {

// These exist properly c++ compliant.  In particular, constexpr-time
// user-defined conversion is UB.  That said, CLang and GCC newer than ~7
// support it.

template <typename Enum>
constexpr bool is_set(const detail::flags<Enum>& f)
{
    return f.is_set();
}

template <typename Enum>
constexpr Enum to_enum(const detail::flags<Enum>& f)
{
    return f.value();
}

}

}

#if UNUSED
// Early GCC (5.x) seems to auto convert 'Enum v' to auto-fail SFINAE on non-type template value
// conversion no matter what.  In other words, no version of enable_if when fed detail::flags<Enum>
// ever passes SFINAE so enable_if is always false
#define ESTD_FLAGS_ENABLE_IF(Enum)  \
template <Enum v>   \
struct Enum ## _enable_if : estd::enable_if<estd::internal::flagged(v)>    {};   \
template <Enum v>   \
using Enum ## _enable_if_t = typename Enum ## _enable_if<v>::type;
#endif


// Auto-promotes 'Enum' to flags<Enum> during these operations
#define ESTD_FLAGS(Enum)    \
constexpr estd::detail::v1::flags<Enum> operator~(Enum v)    \
{ return ~estd::detail::v1::flags<Enum>(v); }     \
constexpr estd::detail::v1::flags<Enum> operator^(Enum lhs, Enum rhs)    \
{ return estd::detail::v1::flags<Enum>(lhs) ^ rhs; }     \
constexpr estd::detail::v1::flags<Enum> operator|(Enum lhs, Enum rhs)    \
{ return estd::detail::v1::flags<Enum>(lhs) | rhs; }     \
constexpr estd::detail::v1::flags<Enum> operator&(Enum lhs, Enum rhs)    \
{ return estd::detail::v1::flags<Enum>(lhs) & rhs; }


// I recall we started with this guy.  Won't auto convert to bool and ADL is pretty sloppy
#define ESTD_FLAGS_EXP(Enum)    \
constexpr Enum operator~(Enum v)    \
{ return Enum(~int(v)); }     \
constexpr Enum operator^(Enum lhs, Enum rhs)    \
{ return Enum(int(lhs) ^ int(rhs)); }     \
constexpr Enum operator|(Enum lhs, Enum rhs)    \
{ return Enum(int(lhs) | int(rhs)); }     \
constexpr Enum operator&(Enum lhs, Enum rhs)    \
{ return Enum(int(lhs) & int(rhs)); }

