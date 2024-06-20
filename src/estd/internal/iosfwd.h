#pragma once

#include "stream_flags.h"
#include "fwd/streambuf.h"
#include "locale/fwd.h"

namespace estd {

namespace internal {

using default_locale = locale<internal::locale_code::en_US, encodings::UTF8>;

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

// For our non-standard << overload which takes a functor
// NOTE: Works great, it's time to put him into 'detail' or a special 'detail::tag'
// namespace
struct ostream_functor_tag {};

}

template <class TStreambuf, class TBase>
detail::basic_istream<TStreambuf, TBase>&
    ws(detail::basic_istream<TStreambuf, TBase>& __is);

// TODO: use specific 16/32/64 bit versions depending on architecture
typedef int streampos;
typedef int streamoff;
typedef int streamsize;



}