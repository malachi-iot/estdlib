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

}
