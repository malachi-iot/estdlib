#pragma once

#include "../fwd/common_type.h"
#include "../fwd/ratio.h"
#include "../numeric.h"
#include "../promoted_type.h"
#include "fwd.h"
#include "traits.h"

namespace estd {


// DEBT: Does not yet account for projector, auto-demotes to
// non-projected.  This may be OK since projected unit IIRC
// auto-normalizes on the way out to regular non-projected flavor
template <class Traits1, class Traits2>
struct common_type<
    units::v1::detail::unit<Traits1>,
    units::v1::detail::unit<Traits2>>
{
    using traits1 = Traits1;
    using traits2 = Traits2;
    using period1 = typename Traits1::period;
    using period2 = typename Traits2::period;
    using tag = typename Traits1::tag;
    using projector = typename Traits1::projector;

    static_assert(is_convertible<decltype(traits1::options), int>::value, "Options must be more-or-less of type units::v1::detail::options");
    // DEBT: Need this, grabbing first options and calling it a day is no good
    //static_assert(is_compatible<traits1::options, traits2::options>::value, "");

    // LLVM doesn't like auto casting integer back to 'options' again.  GCC doesn't care.
    static constexpr auto options = static_cast<units::v1::detail::options>(traits1::options);

    static_assert(is_same<tag, typename Traits2::tag>::value, "Unit tags must match");
    //static_assert(is_same<projector, typename Traits2::projector>::value, "Unit projectors must match");

    using rep = common_type_t<typename Traits1::rep, typename Traits2::rep>;
    // DEBT: still sorting out 'permissive' unit mode, so silently not kicking back against precision loss
    // here.  This is consistent with std::chrono::duration behavior
    //using rep = typename promoted_type<typename Traits1::rep, typename Traits2::rep>::type;

    // DEBT: Use estd intmax_t
    static constexpr std::intmax_t gcd_num = internal::gcd<period1::num, period2::num>::value;
    static constexpr std::intmax_t lcm_den = internal::lcm<period1::den, period2::den>::value;

public:
    using period = ratio<gcd_num, lcm_den>;

    // DEBT: Hardcoding to traits1 for the rebind imperfect
    using traits = typename traits1::template rebind<rep, period, units::v1::passthrough<rep>>;
    //using traits = units::v1::detail::traits<rep, ratio_type, tag>;
    using type = units::v1::detail::unit<traits>;
};


namespace units { inline namespace v1 { namespace detail {

template <class Traits1, class Traits2>
constexpr common_type_t<
    unit<Traits1>,
    unit<Traits2>> ct_helper(
    const unit<Traits1>&,
    const unit<Traits2>&);

// DEBT: I'm pretty sure we don't need this is_arithmetic filter, but until I resolve
// duration vs unit squirrelyness (#184 related) I want that safety guarantee.
template <class Traits, class Rep, enable_if_t<is_arithmetic<Rep>::value, int> = 0>
constexpr
    unit<
        typename Traits::template rebind<common_type_t<typename Traits::rep, Rep>>>
ct_helper(const unit<Traits>&, const Rep&);

template <class Traits1, class Traits2>
using is_same_rep_and_period =
    bool_constant<
        is_same<typename Traits1::rep, typename Traits2::rep>::value &&
        is_same<typename Traits1::period, typename Traits2::period>::value>;


template <class TraitsFrom, class TraitsTo>
using is_promotable_rep_and_same_period =
    bool_constant<
        is_safe_arithmetic_conversion<typename TraitsFrom::rep, typename TraitsTo::rep>::value &&
        is_same<typename TraitsFrom::period, typename TraitsTo::period>::value>;

}}}

}
