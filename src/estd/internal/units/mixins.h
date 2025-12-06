#pragma once

#include "common_type.h"
#include "fwd.h"
#include "traits.h"

// 05DEC25
// mixins 100% EXPERIMENTAL at this time, including 'traits' and 'units' below
// Not fully grokking things just yet - I get the CRTP idea but its application to units
// is still a little fuzzy for me
#define FEATURE_ESTD_UNITS_MIXIN 0

namespace estd { namespace internal { namespace units { namespace mixins {

template <class Traits>
using unit = unit_base<
    typename Traits::rep,
    typename Traits::period,
    typename Traits::tag,
    typename Traits::projector>;

// EXPERIMENTAL, not in use
// unit_base is intended consumer, though it would have to change its signature to unit_base<traits>
template <template <class Traits> class Derived, class Traits>
class adder
{
    using value_type = Derived<Traits>;
    using pointer = value_type*;
    using reference = value_type&;

protected:
    constexpr adder() = default;
public:

    value_type& operator +=(const value_type& v)
    {
        auto self = static_cast<pointer>(this);

        self->rep_ += v.rep_;
        return *self;
    }

    template <class Traits2>
    value_type& operator +=(const Derived<Traits2>& v)
    {
        using CT = decltype(ct_helper(*this, v));
        static_assert(is_same<CT, value_type>::value, "Using += this way would result in precision loss");

        return operator +=(value_type(v));
    }

    template <class Traits2>
    constexpr friend auto operator+(value_type lhs, const Derived<Traits2>& rhs) ->
        decltype(ct_helper(lhs, rhs))
    {
        using CT = decltype(ct_helper(lhs, rhs));

        return CT(lhs) + CT(rhs);
    }
};

}}}}
