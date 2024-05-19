#pragma once

#include "string.h"
#include "../../fwd/string_view.h"

// v0 because in progress, not functionall yet
namespace estd { namespace internal { namespace impl { namespace v0 {

template <class Char>
using out_sviewbuf = out_stringbuf<basic_string_view<Char>>;

template <class Char>
using basic_sviewbuf = basic_stringbuf<basic_string_view<Char>>;

}}}}
