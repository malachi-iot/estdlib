#pragma once

#include "../fwd/ratio.h"

#include "concepts.h"
#include "common_type.h"
#include "fwd.h"

#include "../macro/push.h"

namespace estd { namespace internal { namespace units {

// Indicates to unit_base that narrowing might happen and to silently
// permit it
struct relaxed_narrow_t {};

// DEBT: Pretty sure there's a std/estd flavor of this we can use,
// though our flavor provides extra value in that reflected 'type'
// is important for consumer to use for precision/signing
template <typename Int, Int add>
struct subtractor : estd::integral_constant<Int, add>
{
    template <typename Int2>
    constexpr Int operator()(Int2 v) const
    {
        return v - add;
    }

    // DEBT: Sloppy, but less sloppy than slapping negative signs everywhere
    // else.
    using reversal = adder<Int, add>;
};

template <typename Int, Int add>
struct adder : estd::integral_constant<Int, add>
{
    template <typename Int2>
    constexpr Int operator()(Int2 v) const
    {
        return v + add;
    }

    // DEBT: Sloppy, but less sloppy than slapping negative signs everywhere
    // else.
    using reversal = subtractor<Int, add>;
};





template <class ToScalar, class FromScalar>
constexpr ToScalar unit_cast(const FromScalar& s)
{
    typedef typename ToScalar::rep rep;
    typedef typename ToScalar::period period;

    return cast_helper<rep, period>::do_cast(s);
}

template <class Tag1, class Tag2>
struct compound_tag
{
    using tag1_type = Tag1;
    using tag2_type = Tag2;
};

// EXPERIMENTAL
// Playing with a specialization-generated rate/compound unit (aka mph, kph, mAh, etc)
template <class TUnit1, class TUnit2,
    typename Rep = typename estd::promoted_type<
        typename TUnit1::rep,
        typename TUnit2::rep>::type >
struct compound_unit_helper
{
    using tag_type = compound_tag<
        typename TUnit1::tag_type,
        typename TUnit2::tag_type>;

    // FIX: Need to grab rep/period from somewhere.  Really this ought to be deduced
    // by looking at the lcd/gcd characteristics of both rep/period from both types
    using rep = Rep;
    using period = typename TUnit1::period;

    // TODO: Wrestle with f/offset-er later
    using type = unit_base<rep, period, tag_type>;
};

// EXPERIMENTAL
template <class Unit>
struct unit_traits
{
    using value_type = typename Unit::rep;
    using limits = estd::numeric_limits<value_type>;

    static constexpr value_type min()
    {
        return limits::min();
    }

    static constexpr value_type max()
    {
        return limits::max();
    }
};


// DEBT: Consolidate this with chrono if we can.  Specifically, I don't want disperate
// scalar bases intermingling with one another, so we need some kind of type lockout/forced
// conversion
///
/// @tparam Rep core unit size of underlying count/ticks
/// @tparam Period scaling ratio
/// @tparam Tag differentiating tag so as to disallow one unit from automatically converting to another
/// @tparam F final conversion.  defaults to passthrough (noop)
template <typename Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F>
class unit_base :
    public unit_base_tag,
    public Tag        // Deriving from tag not necessary, but might be useful for is_base_of query
{
protected:
    Rep rep_;

    using traits_type = unit_traits<unit_base>;

#if UNIT_TESTING
public:
#endif

    // It does work, but it is prone to overflow so be careful.  Also,
    // chrono one is supposed to offer compile time protection against overflow
    // and it doesn't, so that's debt/FIX too
    // DEBT: Swap Rep2 and Period2 so that we can auto deduce Rep2
    template <class Rep2, class Period2>
    static constexpr Rep convert_from(const Rep2& count)
    {
        typedef estd::ratio_divide<Period2, Period> rd;
        // find common type so that we more often have the precision we need.
        // still doesn't fully protect us from overflows though (we'd need to go
        // one higher precision for that)
        using ct = estd::common_type_t<Rep, Rep2>;

        // Offset logic was designed to shift "native" j1939 type output
        // to human-friendly form.  In this case though we sometimes go from
        // human-friendly back to "native" so we have to reverse it all.

        // DEBT: It's likely we hit some narrowing conversion situations here,
        // we prefer not to implicitly ignore that as a compiler feature, but rather
        // explicitly ignore it with some kind of indication elsewhere that narrowing happened

        // DEBT: Not quite apples-to-apples, in-place *= & /= don't enjoy type promotion.
        // Causes occasional issues (namely with embr::word), so disabling for now
#if FEATURE_ESTD_DEBUG_CONVERT_FROM &&  __cpp_constexpr >= 201304L   // "relaxed constexpr" (just to make debugging easier)
        auto intermediate = static_cast<ct>(count);
        constexpr typename F::reversal f;
        intermediate *= rd::num;
        intermediate /= rd::den;
        return f(intermediate);
#else
        return typename F::reversal{}(
            ct(count) * rd::num / rd::den);
#endif
    }

    template <class Rep2, class Period2, ESTD_CPP_CONCEPT(Adder<Rep2>) F2>
    static constexpr Rep convert_from(const unit_base<Rep2, Period2, Tag, F2>& v)
    {
        return convert_from<decltype(v.count()), Period2>(v.count());
    }

protected:
    constexpr unit_base() = default;

public:
    explicit constexpr unit_base(const Rep& rep) : rep_{rep} {}

    // Converting constructors are NOT explicit, since we happily want silent conversions
    // in this case.  We're not converting strings etc, but very narrowly similar unit_bases.  See:
    // https://stackoverflow.com/questions/66382983/how-do-i-enable-conversion-from-one-class-to-another
    // https://www.reddit.com/r/cpp_questions/comments/ndnrp0/should_every_singleargument_constructor_be_marked/

    // Converting only precision or F modified
    template <class Rep2, ESTD_CPP_CONCEPT(Adder<Rep2>) F2>
    constexpr unit_base(const unit_base<Rep2, Period, Tag, F2>& s) :    // NOLINT
        rep_{s.count()}
    {
    }

    template <class Rep2, class Period2, ESTD_CPP_CONCEPT(Adder<Rep2>) F2>
    constexpr unit_base(const unit_base<Rep2, Period2, Tag, F2>& s) :   // NOLINT
        rep_{convert_from(s)}
    {
    }

    template <class Rep2, class Period2, ESTD_CPP_CONCEPT(Adder<Rep2>) F2>
    constexpr unit_base(const unit_base<Rep2, Period2, Tag, F2>& s, relaxed_narrow_t) :   // NOLINT
        rep_{Rep(convert_from(s))}
    {
    }

    template <class Rep2, class Period2, class F2>
    unit_base& operator=(const unit_base<Rep2, Period2, Tag, F2>& copy_from)
    {
        rep_ = convert_from(copy_from);
        return *this;
    }

    using f_rep = typename F::value_type;
    using rep = f_rep;
    typedef Rep root_rep;
    //typedef Rep rep;

    typedef Period period;
    typedef Tag tag_type;

    // DEBT: Clean up name here, and document difference between so-called 'root' and regular
    constexpr root_rep root_count() const { return rep_; }
    // DEBT: Hopefully we can get rid of this mutator, brought in while transitioning unit_base
    // to have less implicit behaviors
    void root_count(root_rep v) { rep_ = v; }

    constexpr f_rep count() const { return F{}(rep_); }

    // EXPERIMENTAL
    template <class TCompountUnit>
    using per = typename compound_unit_helper<unit_base, TCompountUnit>::type;

    // For more exotic cases, see standalone operator==() in operators.hpp
    constexpr bool operator==(const unit_base& compare_to) const
    {
        return rep_ == compare_to.rep_;
    }

    unit_base& operator +=(const unit_base& v)
    {
        rep_ += v.rep_;
        return *this;
    }

    template <class Rep2, class Period2, class F2>
    unit_base& operator +=(const unit_base<Rep2, Period2, tag_type, F2>& v)
    {
        using CT = decltype(ct_helper(*this, v));
        static_assert(is_same<CT, unit_base>::value, "Using += this way would result in precision loss");

        return operator +=(unit_base(v));
    }

    template <class Rep2, class Period2, class F2>
    unit_base& operator -=(const unit_base<Rep2, Period2, tag_type, F2>& v)
    {
        using CT = decltype(ct_helper(*this, v));
        static_assert(is_same<CT, unit_base>::value, "Using -= this way would result in precision loss");

        return operator -=(unit_base(v));
    }


    // EXPERIMENTAL - may have diminished/confusing utility especially for
    // floating point types
    unit_base& operator ++()
    {
        ++rep_;
        return *this;
    }

    constexpr unit_base operator -() const
    {
        return unit_base(-rep_);
    }
};


}}}

#include "../macro/pop.h"
