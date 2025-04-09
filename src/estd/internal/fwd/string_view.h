#pragma once

#include "../../policy/string.h"
#include "../../traits/char_traits.h"

// DEBT: Remove this dependency
#include "string.h" // NOLINT

namespace estd {

namespace detail {

// NOTE: One could potentially abuse this and do a non-const policy
template <ESTD_CPP_CONCEPT(internal::StringPolicy) Policy>
class basic_string_view;

}

// NOTE: Because this is marked as a 'const' string policy, resize-ish operations
// are not as available (thus affecting remove_suffix).  Might be prudent to make
// a special 'view' policy which is mostly const, but permits changes to size/pointer
template <class Char,
    class Traits = estd::char_traits<typename estd::remove_const<Char>::type>,
    class Policy = internal::sized_string_policy<Traits, size_t, true> >
using basic_string_view = detail::basic_string_view<Policy>;

#if __cpp_concepts
namespace concepts { inline namespace v1 {

// See https://en.cppreference.com/w/cpp/string/basic_string/operator%3D
template <class T, class Char, class Traits>
concept StringViewLike =
    estd::is_convertible_v<const T&, basic_string_view<Char, Traits> > &&
    estd::is_convertible_v<const T&, const Char*> == false;

}}
#endif

}
