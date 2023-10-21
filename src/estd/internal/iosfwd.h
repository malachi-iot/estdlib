#pragma once

#include "stream_flags.h"
#include "raw/iosfwd.h"

namespace estd {

namespace internal {

template <class TStreambuf,
        estd::internal::stream_flags::flag_type flags = estd::internal::stream_flags::_default>
struct ios_base_policy;

template<class TStreambuf, bool use_pointer = false,
        class TPolicy = ios_base_policy<TStreambuf> >
class basic_ios;

}

namespace detail {

template <class TStreambuf, class TBase = internal::basic_ios<TStreambuf> >
class basic_ostream;

template <class TStreambuf, class TBase = internal::basic_ios<TStreambuf> >
class basic_istream;

}
    
namespace internal {

// Sorry c++03, for you this is a breaking change
#if __cpp_alias_templates
template <class TStreambuf, class TBase = basic_ios<TStreambuf> >
using basic_ostream = detail::basic_ostream<TStreambuf, TBase>;

template <class TStreambuf, class TBase = basic_ios<TStreambuf> >
using basic_istream = detail::basic_istream<TStreambuf, TBase>;

#endif

// For our non-standard << overload which takes a functor
struct ostream_functor_tag {};

}

template <class TStreambuf, class TBase>
detail::basic_istream<TStreambuf, TBase>&
    ws(detail::basic_istream<TStreambuf, TBase>& __is);

}