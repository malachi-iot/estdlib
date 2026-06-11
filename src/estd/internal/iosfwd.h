#pragma once

#include "stream_flags.h"
#include "fwd/streambuf.h"
#include "locale/fwd.h"
#include "istream/fwd.h"
#include "raw/iosfwd.h"

namespace estd {

namespace internal {

// aka "C" locale, "classic" locale
// See https://pubs.opengroup.org/onlinepubs/009604499/basedefs/xbd_chap07.html
using default_locale = locale<internal::locale_codes::en_US, encodings::ASCII>;

template <class Streambuf,
    // DEBT: Deviation, I think std defaults to 'C'/ ASCII ("classic" locale) but
    // IIRC it's not a hard spec requirement
    class Locale = default_locale,
    internal::stream_flags::flag_type flags = estd::internal::stream_flags::_default>
struct ios_base_policy;

// eventually, depending on layering, we will use a pointer to a streambuf or an actual
// value of streambuf itself
template <class Streambuf, bool use_pointer>
class basic_ios_base;


template<ESTD_CPP_CONCEPT(concepts::v1::impl::StreambufBase) Streambuf, bool use_pointer = false,
        class Policy = ios_base_policy<Streambuf> >
class basic_ios;

}

namespace detail {

template <ESTD_CPP_CONCEPT(concepts::v1::OutStreambuf) Streambuf, class Base = internal::basic_ios<Streambuf> >
class basic_ostream;

template <ESTD_CPP_CONCEPT(concepts::v1::InStreambuf) Streambuf, class Base = internal::basic_ios<Streambuf> >
class basic_istream;

// https://github.com/malachi-iot/estdlib/issues/31
// NOTE: this class depends on basic_istream and basic_ostream both of which enforce c++20 concepts,
// so we elect not to
template <class Streambuf>
class basic_iostream;

}
    
namespace internal {

// NOTE: Deprecated, use detail flavor
using ostream_functor_tag = detail::ostream_functor_tag;

}

template <class TStreambuf, class TBase>
detail::basic_istream<TStreambuf, TBase>&
    ws(detail::basic_istream<TStreambuf, TBase>& __is);

}
