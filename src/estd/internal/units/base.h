#pragma once

#include "../fwd/ratio.h"

#include "concepts.h"
#include "common_type.h"
#include "fwd.h"
#include "traits.h"
#include "projectors.h"
#include "scalar.h"

#include "../macro/push.h"

namespace estd { namespace internal { namespace units {

/*
template <class ToScalar, class FromScalar>
constexpr ToScalar unit_cast(const FromScalar& s)
{
    typedef typename ToScalar::rep rep;
    typedef typename ToScalar::period period;

    return cast_helper<rep, period>::do_cast(s);
}*/

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

}}}

namespace estd { namespace units { inline namespace v1 {

// Specifically placing this here and NOT chrono, proclaiming units namespace as the authority.
// Also, we have tag awareness helping specialize for the particular unit in question.
template <class Rep, class Tag = void>
struct treat_as_floating_point : is_floating_point<Rep> {};

template <class ToUnit, class Traits>
constexpr ToUnit unit_cast(const detail::unit<Traits>& u)
{
    return ToUnit(u, relaxed_narrow_t{});
}

namespace detail {


// DEBT: Consolidate this with chrono if we can.  Specifically, I don't want disperate
// scalar bases intermingling with one another, so we need some kind of type lockout/forced
// conversion
// TODO: Look into CRTP https://en.cppreference.com/w/cpp/language/crtp.html to
// aid with member operator implementation (https://github.com/malachi-iot/estdlib/issues/156 )
///
/// @tparam Rep core unit size of underlying count/ticks
/// @tparam Period scaling ratio
/// @tparam Tag differentiating tag so as to disallow one unit from automatically converting to another
/// @tparam F final conversion.  defaults to passthrough (noop)
template <class Traits>
class unit :
    scalar_base<Traits>,
    public unit_base_tag,
    public Traits::tag        // Deriving from tag not necessary, but might be useful for is_base_of query
{
    using base_type = scalar_base<Traits>;

public:
    using traits = Traits;
    using projector = typename Traits::projector;
    using f_rep = typename projector::value_type;
    using rep = f_rep;
    //using rep = typename Traits::rep;
    using period = typename Traits::period;
    using tag = typename Traits::tag;
    using root_rep = typename Traits::rep;

protected:
    template <class Traits2>
    static constexpr bool tag_matches() { return is_same<tag, typename Traits2::tag>::value; }

    template <class Traits2>
    static constexpr bool period_matches() { return is_same<period, typename Traits2::period>::value; }

    using base_type::rep_;

#if UNIT_TESTING
public:
#endif
    static constexpr bool permissive = traits::options & detail::permissive;

    // Splitting out in case we feel like upgrading options to support it.  Not doing so
    // at the moment since rep already appears to be extremely permissive
    static constexpr bool permissive_rep = permissive;
    static constexpr bool permissive_period = permissive;

    // Feeder for unit -> unit converting constructor
    // Matches condition 4 here https://en.cppreference.com/w/cpp/chrono/duration/duration.html
    template <class Traits2>
    static constexpr bool can_unit_convert()
    {
        using rep2 = typename Traits2::rep;

        return
            tag_matches<Traits2>() &&
            is_convertible<rep, rep2>::value &&
            (treat_as_floating_point<rep, tag>::value || permissive_period ||
                (ratio_divide<typename Traits2::period, period>::den == 1 &&
                    !treat_as_floating_point<rep2, tag>::value));
    }


    // It does work, but it is prone to overflow so be careful.  Also,
    // chrono one is supposed to offer compile time protection against overflow
    // and it doesn't, so that's debt/FIX too
    // DEBT: Swap Rep2 and Period2 so that we can auto deduce Rep2
    template <class Rep2, class Period2>
    static constexpr rep convert_from(const Rep2& count)
    {
        using rd = estd::ratio_divide<Period2, period>;
        // find common type so that we more often have the precision we need.
        // still doesn't fully protect us from overflows though (we'd need to go
        // one higher precision for that)
        using ct = estd::common_type_t<rep, Rep2>;

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
        constexpr typename projector::reversal f;
        intermediate *= rd::num;
        intermediate /= rd::den;
        return f(intermediate);
#else
        return typename projector::reversal{}(
            ct(count) * rd::num / rd::den);
#endif
    }

    // May need to enforce 'tag' matches, though callers do that for us
    template <class Traits2>
    static constexpr rep convert_from(const unit<Traits2>& v)
    {
        // These work OK, just taking a hands-off approach until we sort out permissive/strict modes
        //static_assert(is_convertible<decltype(v.count()), rep>::value);
        //static_assert(is_convertible<typename Traits2::rep, rep>::value);

        return convert_from<decltype(v.count()), typename Traits2::period>(v.count());
    }

    // 02FEB26 MB DEBT: I am not seeing any conditions where !permissive kicks back, seems everyone silently
    // converts to 'rep'.  Furthermore, this may REQUIRE permissive behaviors otherwise we can't comfortably
    // do things like unit<int16_t> v(10)

    explicit constexpr unit(const rep& r, false_type) :
        base_type(r) {}

    template <class Rep>
    explicit constexpr unit(const Rep& r, true_type) :
        base_type(rep(r))
    {
    }


public:
    constexpr unit() = default;

    template <class Rep, enable_if_t<is_convertible<const Rep&, rep>::value, int> = 0>
    explicit constexpr unit(const Rep& r) : unit(r, bool_constant<permissive_rep>{})
    {}

    template <class Rep, enable_if_t<is_convertible<const Rep&, rep>::value, int> = 0>
    explicit constexpr unit(const Rep& r, relaxed_narrow_t) : unit(r, true_type{})
    {}


    // Converting constructors are NOT explicit, since we happily want silent conversions
    // in this case.  We're not converting strings etc, but very narrowly similar unit_bases.  See:
    // https://stackoverflow.com/questions/66382983/how-do-i-enable-conversion-from-one-class-to-another
    // https://www.reddit.com/r/cpp_questions/comments/ndnrp0/should_every_singleargument_constructor_be_marked/

    /*
    // Converting only precision or F modified
    template <class Traits2, class = enable_if_t<tag_matches<Traits2>() && period_matches<Traits2>()>>
    constexpr unit_base(const unit_base<Traits2>& s) :    // NOLINT
        rep_{s.count()}
    {
    }   */

    //template <class Traits2, class = enable_if_t<tag_matches<Traits2>() && !period_matches<Traits2>()>>
    template <class Traits2, class = enable_if_t<can_unit_convert<Traits2>()>>
    constexpr unit(const unit<Traits2>& s) :   // NOLINT
        base_type{convert_from(s)}
    {
    }

    template <class Traits2, class = enable_if_t<tag_matches<Traits2>()>>
    constexpr unit(const unit<Traits2>& s, relaxed_narrow_t) :   // NOLINT
        base_type(rep(convert_from(s)))
    {
    }

    template <class Traits2, class = enable_if_t<can_unit_convert<Traits2>()>>
    unit& operator=(const unit<Traits2>& copy_from)
    {
        rep_ = convert_from(copy_from);
        return *this;
    }

    //typedef Rep rep;

    /// retrieve unprojected count()
    constexpr root_rep root_count() const { return rep_; }
    // DEBT: Hopefully we can get rid of this mutator, brought in while transitioning unit_base
    // to have less implicit behaviors
    void root_count(root_rep v) { rep_ = v; }

    constexpr f_rep count() const { return projector{}(rep_); }

    // EXPERIMENTAL
    template <class TCompountUnit>
    using per = typename internal::units::compound_unit_helper<unit, TCompountUnit>::type;

    // For more exotic cases, see standalone operator==() in operators.hpp
    constexpr bool operator==(const unit& compare_to) const
    {
        return rep_ == compare_to.rep_;
    }

    unit& operator +=(const unit& v)
    {
        rep_ += v.rep_;
        return *this;
    }

    unit& operator -=(const unit& v)
    {
        rep_ -= v.rep_;
        return *this;
    }

    template <class Traits2, class = enable_if_t<tag_matches<Traits2>()>>
    unit& operator +=(const unit<Traits2>& v)
    {
        static constexpr bool compatible = permissive || is_promotable_rep_and_same_period<Traits2, Traits>::value;
        static_assert(compatible, "Using += this way would result in precision loss");

        return operator +=(unit(v));
    }

    template <class Traits2, class = enable_if_t<tag_matches<Traits2>()>>
    unit& operator -=(const unit<Traits2>& v)
    {
        static constexpr bool compatible = permissive || is_promotable_rep_and_same_period<Traits2, Traits>::value;
        static_assert(compatible, "Using -= this way would result in precision loss");

        return operator -=(unit(v));
    }

    unit& operator *=(const rep& v)
    {
        rep_ *= v;
        return *this;
    }

    unit& operator /=(const rep& v)
    {
        rep_ /= v;
        return *this;
    }


    // EXPERIMENTAL - may have diminished/confusing utility especially for
    // floating point types
    unit& operator ++()
    {
        ++rep_;
        return *this;
    }

    constexpr const unit operator -() const
    {
        return unit(-rep_);
    }
};

}   // detail

}}} // estd::units::inline v1

#include "../macro/pop.h"
