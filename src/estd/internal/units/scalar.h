#pragma once

#include "../raw/type_traits.h"

#include "enum.h"

namespace estd { namespace units { inline namespace v1 { namespace detail {

template <class Traits, class = void>
class scalar_base;

template <class Traits>
class scalar_base<
    Traits,
    enable_if_t<
        (Traits::options & options::default_mask) == options::default_prohibited ||
        (Traits::options & options::default_mask) == options::none>>
{
public:
    using rep = typename Traits::rep;

protected:
    rep rep_;

    scalar_base() = delete;
    constexpr scalar_base(rep v) : rep_{v}   {}
};


template <class Traits>
class scalar_base<Traits, enable_if_t<(Traits::options & options::default_mask) == options::value_initialized>>
{
public:
    using rep = typename Traits::rep;

protected:
    rep rep_;

    constexpr scalar_base() : rep_{Traits::default_value()} {}
    constexpr scalar_base(rep v) : rep_{v}   {}
};


template <class Traits>
class scalar_base<Traits, enable_if_t<(Traits::options & options::default_mask) == options::default_initialized>>
{
public:
    using rep = typename Traits::rep;

protected:
    rep rep_;

    constexpr scalar_base() = default;
    constexpr scalar_base(rep v) : rep_{v}   {}
};


}}}}
