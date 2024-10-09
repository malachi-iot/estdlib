#pragma once

#include "../fwd/common_type.h"
#include "../fwd/ratio.h"
#include "../numeric.h"
#include "../promoted_type.h"
#include "fwd.h"

namespace estd {

// for units to be a common type, we need the same denominator.  This means
// one of the ratio's numerators must increase, thus reducing the precision on the
// other ratio.
// custom "promoted_type" is utilized to keep duration's 'Rep' from bloating
template <typename Dur1Int, typename Dur2Int,
    std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2,
    // DEBT: Add support for adders here, I don't recall if they modify precision or not
    class Tag>
struct common_type<
    internal::units::unit_base<Dur1Int, ratio<Num1, Denom1>, Tag>,
    internal::units::unit_base<Dur2Int, ratio<Num2, Denom2>, Tag> >
{
private:
    // gracefully promote (or not) types used.  non-specialized common_type is very
    // aggressive about promoting and almost always adds bits - otherwise we'd use it
    using common_int_type = promoted_type<Dur1Int, Dur2Int>::type;

    static constexpr std::intmax_t gcd_num = internal::gcd<Num1, Num2>::value;
    static constexpr std::intmax_t lcm_den = internal::lcm<Denom1, Denom2>::value;

public:
    using ratio_type =  estd::ratio<gcd_num, lcm_den>;

    using type = internal::units::unit_base<common_int_type, ratio_type, Tag>;
};

namespace internal { namespace units {

template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr common_type_t<
    unit_base<Rep1, Period1, Tag, Adder1>,
    unit_base<Rep2, Period2, Tag, Adder2>> ct_helper(
    const unit_base<Rep1, Period1, Tag, Adder1>&,
    const unit_base<Rep2, Period2, Tag, Adder2>&)
{
    return {};
}

}}

}
