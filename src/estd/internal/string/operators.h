#pragma once

#include "../fwd/string.h"

namespace estd {

template <class Impl, class T>
detail::basic_string<Impl>& operator+=(detail::basic_string<Impl>& lhs, T rhs)
{
    lhs.append(rhs);
    return lhs;
}

// DEBT: Grab CharT right from Impl, this is a little too permissive
template <typename CharT, ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
constexpr bool operator ==(const CharT* lhs, const detail::basic_string<Impl>& rhs)
{
    return rhs.compare(lhs) == 0;
}

template <class Impl1, class Impl2>
constexpr bool operator ==(
    const detail::basic_string<Impl1>& lhs,
    const detail::basic_string<Impl2>& rhs)
{
    return lhs.compare(rhs) == 0;
}

}
