#pragma once

#include "../../../traits/char_traits.h"

namespace estd { namespace internal { namespace impl {

template <class Char, class CharTraits = estd::char_traits<Char> >
struct basic_streambuf;

template <class Traits>
struct streambuf_base;

// NOTE: Temporarily making a 'complete' type, but plan is to make this always specialized
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf;


}}}