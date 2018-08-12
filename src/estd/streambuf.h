// Pulled in from util.embedded, but not yet adapted to estd environment

#pragma once

#include "internal/platform.h"
#include "traits/char_traits.h"
#include "type_traits.h"
#include "port/streambuf.h"
#include "internal/ios.h"
#include "internal/impl/streambuf.h"


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



// layer3::basic_streambuf is no longer an acceptable name.  Something at least like
// internal::streambuf since it is fed a TImpl to define what variety of streambuf
// it ends up being, even if it ends up feeding stock-standard virtualized basic_streambuf
namespace layer3 {

// TODO: Phase out TStream and rely totally on TImpl
template<class TChar, class TStream, class Traits = ::std::char_traits <TChar>, class TImpl = estd::internal::impl::native_streambuf<TChar, TStream, Traits>>
class basic_streambuf : public TImpl
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
