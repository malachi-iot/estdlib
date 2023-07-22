#pragma once

#include "../../policy/string.h"
#include "../../traits/char_traits.h"
#include "string.h" // NOLINT

namespace estd {

template <class CharT,
    class Traits = estd::char_traits<typename estd::remove_const<CharT>::type>,
    class Policy = experimental::sized_string_policy<Traits, int16_t, true> >
class basic_string_view;

}