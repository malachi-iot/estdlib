#pragma once

#include "../fwd/common_type.h"
#include "../fwd/ratio.h"
#include "../numeric.h"
#include "../promoted_type.h"
#include "fwd.h"
#include "traits.h"

namespace estd {


template <class Traits1, class Traits2>
struct common_type<
    internal::units::v2::unit_base<Traits1>,
    internal::units::v2::unit_base<Traits2>>
{
    using period1 = typename Traits1::period;
    using period2 = typename Traits2::period;
    using tag = typename Traits1::tag;

    static_assert(is_same<tag, typename Traits2::tag>::value, "Unit tags must match");

    using common_rep_type = typename promoted_type<typename Traits1::rep, typename Traits2::rep>::type;

    // DEBT: Use estd intmax_t
    static constexpr std::intmax_t gcd_num = internal::gcd<period1::num, period2::num>::value;
    static constexpr std::intmax_t lcm_den = internal::lcm<period1::den, period2::den>::value;

public:
    using ratio_type =  ratio<gcd_num, lcm_den>;

    using type = internal::units::unit_base<common_rep_type, ratio_type, tag>;
};


namespace internal { namespace units {

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
        ::estd::units::v1::detail::traits<
            common_type_t<typename Traits::rep, Rep>,
        typename Traits::period, typename Traits::tag, typename Traits::projector>>
    ct_helper(const v2::unit_base<Traits>&, const Rep&)
{
    return {};
}


}}

}
