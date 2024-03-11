#pragma once

#include "streambuf.h"
#include "internal/ios.h"
#include "port/streambuf.h"


namespace estd {

// perhaps do away with traditional basic_ios
// altogether, or make it always the traditional virtualized streambuf flavor
// currently, temporarily, everything is hardwired to the posix_streambuf
// but we've already started to architect that away
#ifdef FEATURE_POSIX_IOS
#ifdef __cpp_alias_templates
template<class TChar, class Traits = ::std::char_traits<TChar> >
using basic_ios = estd::internal::basic_ios<posix_streambuf <TChar, Traits> >;
#endif

// prep for traditional version, but basic_ios_base doesn't yet support non-'stream'
// streambuf
//template<class TChar, class Traits = ::std::char_traits<TChar> >
//using basic_ios = estd::internal::basic_ios<basic_streambuf <TChar, Traits> >;
#else
#endif

//typedef
//basic_ios<char> ios;

// NOTE: These deviate from spec in thair attachment to ostream - necessary due to lack of virtual functions
// Placing here in ios.h anyway just for discoverability and also with hopes c++20 concepts slightly
// decouple us from ostream (though we may never care)

template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& dec(detail::basic_ostream<Streambuf, Base>& out)
{
    out.setf(ios_base::dec, ios_base::basefield);
    return out;
}

template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& hex(detail::basic_ostream<Streambuf, Base>& out)
{
    out.setf(ios_base::hex, ios_base::basefield);
    return out;
}

#if FEATURE_ESTD_OSTREAM_OCTAL
template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& oct(detail::basic_ostream<Streambuf, Base>& out)
{
    out.setf(ios_base::oct, ios_base::basefield);
    return out;
}
#endif

}
