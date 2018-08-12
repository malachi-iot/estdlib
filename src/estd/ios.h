// Pulled in from util.embedded, but not yet adapted to estd environment

#pragma once

#include "streambuf.h"
#include "internal/ios.h"


namespace estd {

// TODO: hardwire in more of a explicit 'posix_streambuf' or whatever based on platform, since proper
// std::ios stuff basic_streambuf is a polymorphic base class.  That or perhaps do away with ios
// typedef altogether
template<class TChar, class Traits = ::std::char_traits<TChar> >
using basic_ios = estd::internal::basic_ios<basic_streambuf <TChar, Traits> >;

typedef
basic_ios<char> ios;

}
