#pragma once

// DEBT: Do a proper forward for char_traits -- at the moment we can't due to a possible 'using' scenario
#include "../../traits/char_traits.h"

namespace estd {

namespace internal {

// all the noop/defaults which we expect to hide/overload are now located in impl
// this struct now amounts to a type tag
struct streambuf_baseline { };


// TODO: TPolicy shall specify modes:
// 1. never blocking
// 2. blocking with timeout (including infinite timeout)
// 3. blocking
// Note that consuming istream/ostream may independently implement its own timeout code
// in which case 'never blocking' mode may be utilized for streambuf
// DEBT: Dedup from implementation comments - not doing so yet as we alias out a 'detail' flavor
template<class Impl, class Policy = void>
class streambuf;

}

namespace detail {

#if __cpp_alias_templates
inline namespace v1 {
template<class Impl, class Policy = void>
using streambuf = internal::streambuf<Impl, Policy>;
}
#endif

}

// traditional basic_streambuf, complete with virtual functions
template<class Char, class Traits = estd::char_traits<Char> >
struct basic_streambuf;


}