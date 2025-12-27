#pragma once

#include <estd/iosfwd.h>
#include <estd/locale.h>
#include "../../ios.h"

#if FEATURE_STD_OSTREAM
#include <ostream>
#endif

#include "fwd.h"

namespace estd { namespace internal { namespace units {

template <class Ostream>
struct is_std_ostream : estd::false_type {};

template <class Ostream>
struct is_estd_ostream : estd::false_type {};

#if FEATURE_STD_OSTREAM
template <class Char, class Traits>
struct is_std_ostream<std::basic_ostream<Char, Traits>> : estd::true_type{};
#endif

template <class Impl, class Base>
struct is_estd_ostream<estd::detail::basic_ostream<Impl, Base>> : estd::true_type{};

// EXPERIMENTAL, UNTESTED
template <class Out>
struct ostream_like
{
    static_assert(is_std_ostream<Out>::value || is_estd_ostream<Out>::value);

    Out* out;
};

// EXPERIMENTAL, UNTESTED
template <class Out, class T>
ostream_like<Out> operator <<(ostream_like<Out> out, T&& v)
{
    *out.out << std::forward<T>(v);
    return out;
}

// DEBT: Slightly horrifying kludge for 'double' support in ostream
template <class TStreambuf, class TBase>
estd::detail::basic_ostream<TStreambuf, TBase>& operator <<(
    estd::detail::basic_ostream<TStreambuf, TBase>& out,
    double v)
{
    auto v_ = (int64_t)v;
    auto v_dec = (int64_t)(v * 100) % 100;

    if(v_dec < 0)   v_dec = -v_dec;

    // DEBT: I think ostream is supposed to auto reset to dec, but isn't
    out << estd::dec << v_;
    out << '.';
    if(v_dec < 10) out << '0';
    out << v_dec;

    return out;
}

template <class Tag, class Period, class TStreambuf, class TBase,
    estd::enable_if_t<
        estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix(estd::detail::basic_ostream<TStreambuf, TBase>& out)
{
    out << traits<Tag>::name();
}

template <class Tag, class Period, class TStreambuf, class TBase,
    estd::enable_if_t<
        !estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix(estd::detail::basic_ostream<TStreambuf, TBase>& out)
{
    out << si::traits<Period, Tag>::name() << traits<Tag>::name();
}

template <class Tag, class Period, class TStreambuf, class TBase,
    estd::enable_if_t<
        estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix_abbrev(estd::detail::basic_ostream<TStreambuf, TBase>& out)
{
    out << traits<Tag>::abbrev();
}

template <class Tag, class Period, class TStreambuf, class TBase,
    estd::enable_if_t<
        !estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix_abbrev(estd::detail::basic_ostream<TStreambuf, TBase>& out)
{
    out << si::traits<Period, Tag>::abbrev() << traits<Tag>::abbrev();
}

// UNTESTED
template <class Tag, class Period, class Out,
         estd::enable_if_t<
             !estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix_abbrev(ostream_like<Out> out)
{
    *out.out << si::traits<Period, Tag>::abbrev() << traits<Tag>::abbrev();
}

template <class Rep, class Period, class F, class Tag, class TStreambuf, class TBase>
void write(estd::detail::basic_ostream<TStreambuf, TBase>& out,
    const unit_base<Rep, Period, Tag, F>& unit)
{
    out << unit.count() << ' ';
    write_suffix<Tag, Period>(out);
}

template <class Rep, class Period, class F, class Tag, class TStreambuf, class TBase>
void write_abbrev(estd::detail::basic_ostream<TStreambuf, TBase>& out,
    const unit_base<Rep, Period, Tag, F>& unit, bool include_space = false)
{
    out << unit.count();
    if(include_space) out << ' ';
    write_suffix_abbrev<Tag, Period>(out);
}

namespace detail {

// DEBT: This guy is great, make concept support 
template <class Unit>
struct unit_put : estd::detail::ostream_functor_tag
{
    const Unit unit;
    const bool abbrev;

    using rep = typename Unit::rep;
    using period = typename Unit::period;
    using tag = typename Unit::tag_type;

    constexpr unit_put(const Unit& unit, bool abbrev) :
        unit{unit},
        abbrev{abbrev}
    {}

    template <class Streambuf, class Base>
    void operator()(estd::detail::basic_ostream<Streambuf, Base>& out) const
    {
        if(abbrev)
            write_abbrev(out, unit);
        else
            write(out, unit);
    }
};

// For ADL to pick this up, it has to live in
// same namespace as unit_put
/*
template <class TUnit, class TStreambuf, class TBase>
estd::detail::basic_ostream<TStreambuf, TBase>& operator <<(
    estd::detail::basic_ostream<TStreambuf, TBase>& out,
    unit_put<TUnit> v)
{
    if(v.abbrev)
        write_abbrev(out, v.unit);
    else
        write(out, v.unit);

    return out;
} */

}


#if FEATURE_STD_OSTREAM
// UNTESTED
template <class Char, class Traits, class Unit>
inline std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& out,
    const detail::unit_put<Unit>& unit)
{
    using tag = typename Unit::tag_type;
    using period = typename Unit::period;

    //out << si::traits<Period, Tag>::name() << traits<Tag>::name();
    //out << traits<Tag>::abbrev();
    out << unit.unit.count();

    out << ' ';

    if(unit.abbrev)
    {
        out << si::traits<period, tag>::abbrev() << traits<tag>::abbrev();
    }
    else
    {
        out << si::traits<period, tag>::name() << traits<tag>::name();
    }

    return out;
}
#endif


}}

template <class Rep, class Period, class Tag, class F>
constexpr internal::units::detail::unit_put<
    internal::units::unit_base<Rep, Period, Tag, F> > put_unit(
    const internal::units::unit_base<Rep, Period, Tag, F>& unit, bool abbrev = true)
{
    return { unit, abbrev };
}

}
