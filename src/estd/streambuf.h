// Pulled in from util.embedded, but not yet adapted to estd environment

#pragma once

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

#include "features.h"
#include "locale.h"

#ifdef FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF
#include "CircularBuffer.h"
#endif

#ifdef FEATURE_FRAB
#include <frab/systime.h>
#endif


extern "C"
{
#include <stdio.h> // For POSIX modes
#include <stdint.h>
}

namespace FactUtilEmbedded { namespace std {


// TODO: find a better home for char_traits and friends.  Normally it lives in <string> -
// it seems likely most platforms would have this already, so check into that and if so, eliminate
// our special version

// C++ spec actually defines streamsize as signed to accomodate some streambuf operations
// which we don't support, so I'm gonna make them unsigned
typedef uint16_t streamoff;
typedef uint16_t streamsize;

#if __cplusplus > 199711L
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif
//  As per http://tuttlem.github.io/2014/08/18/getting-istream-to-work-off-a-byte-array.html
//  I won't try to stuff in a uint8_t as a TChar anywhere, although it seems like I could
// safely make a fully uint8_t version of things

// TODO: move this into our string.h (also this prototype is present in c_types.h)
template <class TChar> struct char_traits;

template <> struct char_traits<char>
{
    typedef char char_type;
#ifdef __MBED__
    typedef int16_t int_type;
#else
    typedef int int_type;
#endif

    static CONSTEXPR char_type to_char_type(int_type ch) { return ch; }
    static CONSTEXPR int_type to_int_type(const char ch) { return ch; }
    static CONSTEXPR int_type eof() { return -1; }
    static CONSTEXPR bool eq(char c1, char c2) { return c1 == c2; }
    static const char_type* find(const char_type* p, size_t count, const char_type& ch)
    {
        while(count--)
        {
            if(*p == ch) return p;
            p++;
        }

        return nullptr;
    }

#ifdef FEATURE_IOS_EXPERIMENTAL_TRAIT_NODATA
    // Non-standard timeout/data unavailable return value, since eof() suggests no more data EVER
    // we want a different error code
    static CONSTEXPR int_type nodata() { return -2; }
#endif
};

namespace layer3 {

template<class TChar, class TStream, class Traits = char_traits <TChar>>
class basic_streambuf
{
protected:
    typedef TChar char_type;
    typedef typename Traits::int_type int_type;
    TStream &stream;

#ifdef FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF
    FactUtilEmbedded::layer1::CircularBuffer<char_type, (uint16_t)FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF> experimental_buf;
#endif

    streamsize xsputn(const char_type *s, streamsize count);

    streamsize xsgetn(char_type *s, streamsize count);

public:
    typedef TStream stream_t;

    basic_streambuf(TStream &stream) : stream(stream)
    {}

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
#elif defined(__POSIX__)
#include "streams/iostream_posix_streambuf.h"
#elif defined(ARDUINO)
#include "streams/iostream_arduino_streambuf.h"
#elif defined(ESP_OPEN_RTOS)
#include "streams/iostream_posix_streambuf.h"
#else
#error "Architecture not yet supported"
#endif
#endif

}}