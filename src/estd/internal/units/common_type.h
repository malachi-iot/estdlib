#pragma once

#include "../fwd/common_type.h"
#include "../fwd/ratio.h"
#include "../numeric.h"
#include "../promoted_type.h"
#include "fwd.h"
#include "traits.h"

namespace estd {


// FIX: Make sure tags match too
template <class Traits1, class Traits2>
struct common_type<
    internal::units::v2::unit_base<Traits1>,
    internal::units::v2::unit_base<Traits2>>
{
    using period1 = typename Traits1::period;
    using period2 = typename Traits2::period;
    using tag = typename Traits1::tag;

    // DEBT: Produce better error message
    static_assert(is_same<tag, typename Traits2::tag>::value, "Tag mismatch");

    using common_rep_type = typename promoted_type<typename Traits1::rep, typename Traits2::rep>::type;

    // DEBT: Use estd intmax_t
    static constexpr std::intmax_t gcd_num = internal::gcd<period1::num, period2::num>::value;
    static constexpr std::intmax_t lcm_den = internal::lcm<period1::den, period2::den>::value;

public:
    using ratio_type =  ratio<gcd_num, lcm_den>;

    using type = internal::units::unit_base<common_rep_type, ratio_type, tag>;
};

// for units to be a common type, we need the same denominator.  This means
// one of the ratio's numerators must increase, thus reducing the precision on the
// other ratio.
// custom "promoted_type" is utilized to keep duration's 'Rep' from bloating
template <typename Dur1Int, typename Dur2Int,
    std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2,
    // DEBT: Add support for projectors here, I believe they do modify precision
    class Tag>
struct common_type<
    internal::units::unit_base<Dur1Int, ratio<Num1, Denom1>, Tag>,
    internal::units::unit_base<Dur2Int, ratio<Num2, Denom2>, Tag> >
{
private:
    // gracefully promote (or not) types used.  non-specialized common_type is very
    // aggressive about promoting and almost always adds bits - otherwise we'd use it
    using common_int_type = typename promoted_type<Dur1Int, Dur2Int>::type;

    static constexpr std::intmax_t gcd_num = internal::gcd<Num1, Num2>::value;
    static constexpr std::intmax_t lcm_den = internal::lcm<Denom1, Denom2>::value;

public:
    using ratio_type =  estd::ratio<gcd_num, lcm_den>;

    using type = internal::units::unit_base<common_int_type, ratio_type, Tag>;
};

namespace internal { namespace units {

// FIX: match on 'tag'
template <class Traits1, class Traits2>
constexpr common_type_t<
    v2::unit_base<Traits1>,
    v2::unit_base<Traits2>> ct_helper(
    const v2::unit_base<Traits1>&,
    const v2::unit_base<Traits2>&)
{
    return {};
}

template <class Traits, class Rep>
constexpr
    v2::unit_base<
        unit_traits<
            common_type_t<typename Traits::rep, Rep>,
        typename Traits::period, typename Traits::tag, typename Traits::projector>>
    ct_helper(const v2::unit_base<Traits>&, const Rep&)
{
    return {};
}


}}

}
