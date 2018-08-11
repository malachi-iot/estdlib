// Pulled in from util.embedded, but not yet adapted to estd environment

#pragma once

#include "internal/platform.h"
#include "traits/char_traits.h"
#include "type_traits.h"

#ifdef ESP_OPEN_RTOS
#elif defined(__MBED__)
#include <drivers/Stream.h>
// looks like a collision
// on the F() macro, so undef it first
#ifdef F
#define F_DEFINED
#undef F
#endif
#include <drivers/Serial.h>
// redefine F here.  Kludgey for sure.  There will be situations where
// F wants to be what Serial.h set it to
#ifdef F_DEFINED
//#include "noduino_F.h"
#undef F_DEFINED
#endif

#elif defined(ARDUINO)
#include <Arduino.h>
#endif

//#include "features.h"
#include "locale.h"

#ifdef FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF
#include "CircularBuffer.h"
#endif

#ifdef FEATURE_FRAB
#include <frab/systime.h>
#endif


#ifdef ESTD_POSIX
extern "C"
{
#include <stdio.h> // For POSIX modes
#include <stdint.h>
}
#endif

namespace estd {


// TODO: find a better home for char_traits and friends.  Normally it lives in <string> -
// it seems likely most platforms would have this already, so check into that and if so, eliminate
// our special version

// C++ spec actually defines streamsize as signed to accomodate some streambuf operations
// which we don't support, so I'm gonna make them unsigned
typedef uint16_t streamoff;
typedef uint16_t streamsize;


namespace layer3 {

template<class TChar, class TStream, class Traits = ::std::char_traits <TChar>>
class basic_streambuf
{
public:
    typedef TChar char_type;
    typedef Traits traits_type;
    typedef typename Traits::int_type int_type;
protected:
    TStream stream;

#ifdef FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF
    FactUtilEmbedded::layer1::CircularBuffer<char_type, (uint16_t)FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF> experimental_buf;
#endif

    streamsize xsputn(const char_type *s, streamsize count);

    streamsize xsgetn(char_type *s, streamsize count);

public:
    // NOTE: we'll need to revisit this if we want a proper pointer in here
    typedef typename estd::remove_reference<TStream>::type stream_type;

    basic_streambuf(stream_type& stream) : stream(stream)
    {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    basic_streambuf(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif

    // http://putka.upm.si/langref/cplusplus.com/reference/iostream/streambuf/sgetn/index.html
    // acts like many sbumpc calls
    streamsize sgetn(char_type *s, streamsize count)
    {
        return xsgetn(s, count);
    }

    streamsize sputn(const char_type *s, streamsize count)
    {
        return xsputn(s, count);
    }


    int_type sputc(char_type ch)
    {
        bool success = xsputn(&ch, sizeof(ch)) == sizeof(ch);
        return success ? Traits::to_int_type(ch) : Traits::eof();
    }


    // TODO: *possibly* implement underflow, if I like it...
    // Don't think I made this one quite right...
    int_type sbumpc()
    {
        char_type ch;

        bool success = xsgetn(&ch, sizeof(ch)) == sizeof(ch);

        return success ? Traits::to_int_type(ch) : Traits::eof();
    }

    int_type sgetc();

#ifdef FEATURE_IOS_SPEEKC
    int_type speekc();
#endif

    // NOTE: this deviates from spec in that it won't wait for CR, for example,
    // to reflect characters are available
    streamsize in_avail();
};
}

#ifdef FEATURE_IOS_STREAMBUF_FULL
template<class TChar, class Traits = char_traits<TChar>>
class basic_streambuf
{
protected:
    typedef TChar char_type;
    typedef typename Traits::int_type int_type;

    virtual streamsize xsputn(const char_type* s, streamsize count) = 0;
    virtual streamsize xsgetn(char_type* s, streamsize count) = 0;

    //virtual int_type uflow() = 0;

public:
    streamsize sputn(const char_type* s, streamsize count)
    {
        return xsputn(s, count);
    }

    int_type sputc(char_type ch)
    {
        bool success = xsputn(&ch, sizeof(ch)) == sizeof(ch);
        return success ? (int_type) ch : Traits::eof();
    }

    streamsize sgetn(char_type* s, streamsize count)
    {
        return xsgetn(s, count);
    }

    // TODO: *possibly* implement underflow, if I like it...
    // Don't think I made this one quite right...
    int_type sbumpc()
    {
        char_type ch;

        bool success = xsgetn(&ch, sizeof(ch)) == sizeof(ch);

        return success ? Traits::to_int_type(ch) : Traits::eof();
    }

    // FIX: make this pure and implement in derived class
    virtual streamsize in_avail() { return 0; }
};
#else
#if defined(__MBED__)
#include "streams/iostream_mbed_streambuf.h"
#elif defined(ESTD_POSIX)
#include "port/posix/streambuf.h"
#elif defined(ARDUINO)
#include "streams/iostream_arduino_streambuf.h"
#elif defined(ESP_OPEN_RTOS)
#include "streams/iostream_posix_streambuf.h"
#else
#error "Architecture not yet supported"
#endif
#endif

}