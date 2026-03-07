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
class converting_adder
{
    using value_type = Derived<Traits>;
    using pointer = value_type*;
    using reference = value_type&;

public:

    value_type& operator +=(const value_type& v)
    {
        auto self = static_cast<pointer>(this);

        self->rep_ += v.rep_;
        return *self;
    }

    // FIX: Hmm this seems wrong, do we really want to return value_type and especially value_type&
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

// EXPERIMENTAL
template <class Derived, class Base>
class wrapped_adder
{
    ESTD_CPP_STD_VALUE_TYPE(typename Base::rep)

public:
    Derived& operator +=(const Derived& v)
    {
        return static_cast<Derived&>(Base::operator +=(v));
    }

    constexpr friend Derived operator+(const Derived& lhs, const Derived& rhs)
    {
        return Derived(lhs + rhs);
    }
};


// EXPERIMENTAL
template <class Derived, class Base>
class wrapped_subtractor
{
    ESTD_CPP_STD_VALUE_TYPE(typename Base::rep)

public:
    Derived& operator -=(const Derived& v)
    {
        return static_cast<Derived&>(Base::operator -=(v));
    }

    constexpr friend Derived operator-(const Derived& lhs, const Derived& rhs)
    {
        return Derived(lhs - rhs);
    }
};


// EXPERIMENTAL
// Probably not needed, only useful if constructor itself adds more utility than detail::units one
template <template <class Traits> class Derived, class Traits>
class unit_compare_gt
{
public:
    template <class Traits2>
    constexpr bool operator >(const Derived<Traits2>& compare_to) const
    {
        auto self = static_cast<const Derived<Traits>*>(this);
        using CT = estd::common_type_t<Derived<Traits>, Derived<Traits2>>;

        return CT(self->count()) > CT(compare_to.count());
    }

    template <class Traits2>
    constexpr bool operator >=(const Derived<Traits2>& compare_to) const
    {
        auto self = static_cast<const Derived<Traits>*>(this);
        using CT = estd::common_type_t<Derived<Traits>, Derived<Traits2>>;

        return CT(self->count()) >= CT(compare_to.count());
    }
};

}}}}
