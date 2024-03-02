#pragma once

#include "../../policy/string.h"
#include "../../traits/char_traits.h"

// DEBT: Remove this dependency
#include "string.h" // NOLINT

namespace estd {

template <class CharT,
    class Traits = estd::char_traits<typename estd::remove_const<CharT>::type>,
    class Policy = experimental::sized_string_policy<Traits, int16_t, true> >
class basic_string_view;

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