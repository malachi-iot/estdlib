#pragma once

#include "stream_flags.h"
#include "fwd/streambuf.h"

namespace estd {

namespace internal {

template <class Streambuf,
        estd::internal::stream_flags::flag_type flags = estd::internal::stream_flags::_default>
struct ios_base_policy;

template<ESTD_CPP_CONCEPT(concepts::v1::impl::StreambufBase) Streambuf, bool use_pointer = false,
        class Policy = ios_base_policy<Streambuf> >
class basic_ios;

}

namespace detail {

template <ESTD_CPP_CONCEPT(concepts::v1::OutStreambuf) Streambuf, class Base = internal::basic_ios<Streambuf> >
class basic_ostream;

template <class Streambuf, class Base = internal::basic_ios<Streambuf> >
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