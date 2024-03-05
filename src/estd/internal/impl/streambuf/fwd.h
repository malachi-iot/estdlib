#pragma once

#include "../../../traits/char_traits.h"
#include "../../fwd/string.h"   // Only for CharTraits concept

namespace estd { namespace internal { namespace impl {

template <class Char, class CharTraits = estd::char_traits<Char> >
struct basic_streambuf;

template <ESTD_CPP_CONCEPT(concepts::v1::CharTraits) Traits, class Signal = void>
struct streambuf_base;

// NOTE: Temporarily making a 'complete' type, but plan is to make this always specialized
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf;


}}}
