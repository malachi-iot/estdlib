#pragma once

#include "../feature/std.h"
#include "../fwd/string.h"

#if FEATURE_STD_OSTREAM
#include <ostream>
#endif

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

// DEBT: std::ostream support should actually be elsewhere

#if FEATURE_STD_OSTREAM
//A bit finicky so that we can remove const (via Traits::char_type)
template <class Char, class Traits, class Impl>
inline std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os,
    const estd::detail::basic_string<Impl>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
}
#endif

// Clang seems to want this pseudo-specialization
// Somehow clang has slightly different expectations during catch << resolution
template <class Char, class Impl>
std::ostream& operator <<(
    std::ostream& os, const estd::detail::basic_string<Impl>& value)
{
    // DEBT: This only works for null terminated
    const char* s = value.clock();
    operator <<(os, s);
    value.cunlock();
    return os;
}

}
