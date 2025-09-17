#pragma once

#include "cstddef.h"

namespace estd {

namespace internal {

struct in_place_tag {};

template <size_t>
struct in_place_conditional_t : in_place_tag {};

}

struct in_place_t : internal::in_place_tag
{
    constexpr explicit in_place_t() = default;
};

template <class T>
struct in_place_type_t : internal::in_place_tag
{
    constexpr explicit in_place_type_t() = default;
};

template <size_t I>
struct in_place_index_t : internal::in_place_tag
{
    // Non-standard, but harmless.  estd::variadic picks this up
    static constexpr size_t index = I;

    constexpr explicit in_place_index_t() = default;
};

#if __cpp_constexpr >= 201304L
template <class T>
constexpr in_place_type_t<T> in_place_type {};

template <size_t I>
constexpr in_place_index_t<I> in_place_index {};
#endif

// DEBT: Not well tested - but it seems GCC can do global constexpr variables sooner than spec allows, so
// take advantage
#if __cplusplus >= 201703L || (defined(__GNUC__) && __cpp_constexpr)
constexpr in_place_t in_place{};
#endif

}
