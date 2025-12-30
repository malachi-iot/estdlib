#pragma once

#include <estd/iosfwd.h>
#include <estd/locale.h>
#include "../../ios.h"

#if FEATURE_STD_OSTREAM
#include <ostream>
#endif

#include "fwd.h"

namespace estd { namespace internal {

template <class>
struct is_std_ostream : estd::false_type {};

template <class>
struct is_estd_ostream : estd::false_type {};

#if FEATURE_STD_OSTREAM
template <class Char, class Traits>
struct is_std_ostream<std::basic_ostream<Char, Traits>> : estd::true_type{};
#endif

template <class Streambuf, class Base>
struct is_estd_ostream<estd::detail::basic_ostream<Streambuf, Base>> : estd::true_type{};

template <class Out>
struct ostream_like
{
    static_assert(is_std_ostream<Out>::value || is_estd_ostream<Out>::value);

    Out* out;

    /* No substitute for CTAD unfortunately
    template <class Char, class Traits>
    constexpr ostream_like(std::basic_ostream<Char, Traits>& out) : out(&out) {}

    template <class Impl, class Base>
    constexpr ostream_like(estd::detail::basic_ostream<Impl, Base>& out) : out(&out)    {}  */

    operator Out&() { return *out; }

    template <class T>
    ostream_like& operator <<(T&& v)
    {
        *out << std::forward<T>(v);
        return *this;
    }
};


/*
template <class Out>
ostream_like<Out> make_ostream_like(Out& out)
{
    return { &out };
}*/

template <class Streambuf, class Base>
ostream_like<estd::detail::basic_ostream<Streambuf, Base>> make_ostream_like(estd::detail::basic_ostream<Streambuf, Base>& out)
{
    return { &out };
}

template <class Char, class Traits>
ostream_like<std::basic_ostream<Char, Traits>> make_ostream_like(std::basic_ostream<Char, Traits>& out)
{
    return { &out };
}

/*
// EXPERIMENTAL, UNTESTED
template <class Out, class T>
ostream_like<Out> operator <<(ostream_like<Out> out, T&& v)
{
    *out.out << std::forward<T>(v);
    return out;
}   */

template <class Tag, class Period, class Out,
    estd::enable_if_t<
        estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix(ostream_like<Out> out)
{
    out << units::traits<Tag>::name();
}

template <class Tag, class Period, class Out,
    estd::enable_if_t<
        !estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix(ostream_like<Out> out)
{
    out << units::si::traits<Period, Tag>::name() << units::traits<Tag>::name();
}

template <class Tag, class Period, class Out,
    estd::enable_if_t<
        estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix_abbrev(ostream_like<Out> out)
{
    *out.out << units::traits<Tag>::abbrev();
}


template <class Tag, class Period, class Out,
    estd::enable_if_t<
        !estd::is_same<Period, estd::ratio<1>>::value, bool> = true>
void write_suffix_abbrev(ostream_like<Out> out)
{
    *out.out << units::si::traits<Period, Tag>::abbrev() << units::traits<Tag>::abbrev();
}

template <class Traits, class Out>
void write(ostream_like<Out> out,
    const estd::units::v1::detail::unit<Traits>& unit)
{
    out << unit.count() << ' ';
    write_suffix<typename Traits::tag, typename Traits::period>(out);
}

template <class Traits, class Out>
void write_abbrev(ostream_like<Out> out,
    const estd::units::v1::detail::unit<Traits>& unit, bool include_space = false)
{
    out << unit.count();
    if(include_space) out << ' ';
    write_suffix_abbrev<typename Traits::tag, typename Traits::period>(out);
}

}

namespace units { inline namespace v1 {

// DEBT: This guy is great, make concept support 
template <class Unit>
struct unit_put : estd::detail::ostream_functor_tag
{
    const Unit unit;
    const bool abbrev;

    using rep = typename Unit::rep;
    using period = typename Unit::period;
    using tag = typename Unit::tag;

    constexpr unit_put(const Unit& unit, bool abbrev) :
        unit{unit},
        abbrev{abbrev}
    {}

    template <class Out>
    void operator()(internal::ostream_like<Out> out) const
    {
        if(abbrev)
            write_abbrev(out, unit);
        else
            write(out, unit);
    }

    template <class Streambuf, class Base>
    void operator()(estd::detail::basic_ostream<Streambuf, Base>& out) const
    {
        return operator()(make_ostream_like(out));
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

//}


#if FEATURE_STD_OSTREAM
template <class Char, class Traits, class Unit>
inline std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& out,
    const unit_put<Unit>& unit)
{
    unit(internal::make_ostream_like(out));

    return out;
}
#endif


template <class Traits>
constexpr unit_put<detail::unit<Traits>> put_unit(
    const detail::unit<Traits>& unit, bool abbrev = true)
{
    return { unit, abbrev };
}

}}

}
