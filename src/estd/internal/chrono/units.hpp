/**
 * C++20 units helpers and friends
 * e.g. https://en.cppreference.com/w/cpp/chrono/year
 */
// DEBT: A lot of chrono code is in port/chrono.h/hpp, would be better if it was instead here
#pragma once

#include "../fwd/chrono.h"
#include "../../port/chrono.hpp"

#include "units.hpp"

namespace estd { namespace chrono {

namespace internal {

// DEBT: Only using these aliases to bring things up.
// Stand-ins for sys_time, sys_seconds, sys_days
template<class Duration, class TClock>
using clock_time = estd::chrono::time_point<TClock, Duration>;

template <class TClock>
using clock_days = clock_time<estd::chrono::days, TClock>;

// Generic int wrapper.  There must be something like this elsewhere already...
// NOTE: We are forward casting all over the place here, which means inheriting class
// SHOULD NOT do multiple inheritance.  Furthermore, this is quite easily broken so
// don't use this class unless you really understand the implications of this freewheeling
// forward casting
template <typename TInt, typename TContainer>
class unit_base
{
protected:
    TInt value_;

    constexpr unit_base(TInt value) : value_{value} {}

public:
    inline TContainer& operator+=(const TContainer& v)
    {
        value_ += v.value_;
        return *(TContainer*)this;
    }

    inline TContainer& operator-=(const TContainer& v)
    {
        value_ -= v.value_;
        return *(TContainer*)this;
    }

    constexpr operator TInt() const { return value_; }
};

}

class year : public internal::unit_base<int16_t, year>
{
    typedef internal::unit_base<int16_t, year> base_type;

public:
    year() = default;
    explicit constexpr year(int year) : base_type{(int16_t)year} {}
};


class month : public internal::unit_base<uint8_t, month>
{
    typedef internal::unit_base<uint8_t, month> base_type;

public:
    month() = default;
    explicit constexpr month(int month) : base_type{(uint8_t)month} {}
};


class day : public internal::unit_base<uint8_t, day>
{
    typedef internal::unit_base<uint8_t, day> base_type;

public:
    day() = default;
    explicit constexpr day(int day) : base_type{(uint8_t)day} {}
};

#ifdef FEATURE_CPP_INLINE_VARIABLES
inline constexpr month January{1};
inline constexpr month February{2};
inline constexpr month March{3};
inline constexpr month April{4};
inline constexpr month May{5};
inline constexpr month June{6};
inline constexpr month July{7};
inline constexpr month August{8};
inline constexpr month September{9};
inline constexpr month October{10};
inline constexpr month November{11};
inline constexpr month December{12};
#endif

}}