#pragma once

// Lightweight ios compatibility for targets (which seem to be many) who don't have an iostream
// implementation.  Also can and should serve as a wrapper class around Stream implementations
// found in libs like Arduino & mbed OS

#include "streambuf.h"
#include "ios.h"
#include "istream.h"
#include "ostream.h"

namespace FactUtilEmbedded { namespace std
{


#ifdef FEATURE_IOS_STREAMBUF_FULL
template <class TChar, class traits = char_traits<TChar>>
class basic_iostream :
        public basic_ostream<TChar, traits>,
        public basic_istream<TChar, traits>
{

};
#else
// don't use virtual inheritance and instead manually redefine basic_istream behaviors
// creates some visual code slop, but ends up compiling smaller (and maybe faster)
template <class TChar, class traits = char_traits<TChar>>
class basic_iostream :
        public basic_ostream<TChar, traits>
{

};
#endif



} }

#ifdef OBSOLETE
#ifdef ESP_OPEN_RTOS
#include "streams/iostream_esp8266.h"
#elif defined(__POSIX__)
#include "streams/iostream_posix.h"
#elif defined(__MBED__)
#include "streams/iostream_mbed.h"
#elif defined(ARDUINO)
#include "streams/iostream_arduino.h"
#else
#warning "Unknown architecture"
#endif
#endif

namespace FactUtilEmbedded { namespace std
{
typedef basic_streambuf<char> streambuf;


// Shamelessly lifted from GCC4.8.5
/**
 *  @brief  Write a newline and flush the stream.
 *
 *  This manipulator is often mistakenly used when a simple newline is
 *  desired, leading to poor buffering performance.  See
 *  http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt11ch25s02.html
 *  for more on this subject.
*/
/*
typedef basic_ostream<_CharT, _Traits>            __ostream_type;

__ostream_type&
      operator<<(__ostream_type& (*__pf)(__ostream_type&))
      {
        // _GLIBCXX_RESOLVE_LIB_DEFECTS
        // DR 60. What is a formatted input function?
        // The inserters for manipulators are *not* formatted output functions.
        return __pf(*this);
      }

template<typename _CharT, typename _Traits>
  inline basic_ostream<_CharT, _Traits>&
  endl(basic_ostream<_CharT, _Traits>& __os)
  { return flush(__os.put(__os.widen('\n'))); }
  */

/*
__ostream_type& operator<<(__ostream_type& (*__pf)(__ostream_type&))
{
    return __pf(*this);
}*/

extern ostream cout;
extern istream cin;
extern ostream& clog;
extern ostream& cerr;

// FIX: Not a great location for this, but has to live somewhere for now
// based on C++ assert() macro specification, but a little wimpier
#ifndef ASSERT
#ifdef DEBUG
//#define ASSERT(condition) if(!(condition)) { cerr << "Failure: " << __FILE__ << __LINE__ << __func__ << endl; }
#define ASSERT(condition, message) \
{   \
    bool evaluated = condition; \
    if(!evaluated) { cerr << __func__ << ": " << message << endl; } \
}
#else
//#define ASSERT(condition)
#define ASSERT(condition, message)
#endif
#endif

} }