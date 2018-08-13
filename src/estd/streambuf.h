// Pulled in from util.embedded, but not yet adapted to estd environment

#pragma once

#include "internal/platform.h"
#include "traits/char_traits.h"
#include "traits/allocator_traits.h" // for ESTD_HAS_METHOD_EXPERIMENTAL
#include "type_traits.h"
// FIX: Temporarily including this at the bottom to satisfy dependencies
//#include "port/streambuf.h"
#include "internal/ios.h"
#include "internal/impl/streambuf.h"


//#include "features.h"
#include "locale.h"

#ifdef FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF
#include "CircularBuffer.h"
#endif

namespace estd {

namespace internal {

#define ESTD_HAS_METHOD_EXPERIMENTAL1(ret_type, method_name, param_type1) \
template <class T> struct has_##method_name##_method : has_member_base \
{ \
    template <typename C> static CONSTEXPR yes& test(reallyHas<ret_type (C::*)(param_type1), &C::method_name>* /*unused*/) \
    { return yes_value; }  \
\
    template <typename> static CONSTEXPR no& test(...) { return no_value; } \
\
    static CONSTEXPR bool value = sizeof(test<T>(0)) == sizeof(yes); \
};

ESTD_HAS_METHOD_EXPERIMENTAL1(int, sputc, char);
ESTD_HAS_METHOD_EXPERIMENTAL(int, sgetc);
ESTD_HAS_METHOD_EXPERIMENTAL(int, sbumpc);

template<class TImpl>
class streambuf : public TImpl
{
    typedef TImpl base_type;
public:
    // estd::internal::impl::native_streambuf<TChar, TStream, Traits
    typedef typename TImpl::char_type char_type;
    typedef typename TImpl::traits_type traits_type;
    typedef typename traits_type::int_type int_type;
protected:

#ifdef FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF
    FactUtilEmbedded::layer1::CircularBuffer<char_type, (uint16_t)FEATURE_IOS_EXPERIMENTAL_STREAMBUFBUF> experimental_buf;
#endif

    streamsize xsputn(const char_type *s, streamsize count)
    {
        return base_type::xsputn(s, count);
    }

    streamsize xsgetn(char_type *s, streamsize count)
    {
        return base_type::xsgetn(s, count);
    }

public:

    template <class Param1>
    streambuf(Param1& p1) : base_type(p1)
    {}

    /*
#ifdef FEATURE_CPP_MOVESEMANTIC
    streambuf(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif */

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

    // Do SFINAE and call TImpl version if present
    template <class T = base_type>
    typename enable_if<has_sputc_method<T>::value, int_type>::type
    sputc(char_type ch)
    {
        return base_type::sputc(ch);
    }

    // if TImpl doesn't have one, use a generic one-size-fits all version
    template <class T = base_type>
    typename enable_if<!has_sputc_method<T>::value, int_type>::type
    sputc(char_type ch)
    {
        bool success = xsputn(&ch, sizeof(ch)) == sizeof(ch);
        return success ? traits_type::to_int_type(ch) : traits_type::eof();
    }


    // Do SFINAE and call TImpl version if present
    template <class T = base_type>
    typename enable_if<has_sbumpc_method<T>::value, int_type>::type
    sbumpc()
    {
        return base_type::sbumpc();
    }

    // TODO: *possibly* implement underflow, if I like it...
    // Don't think I made this one quite right...
    template <class T = base_type>
    typename enable_if<!has_sbumpc_method<T>::value, int_type>::type
    sbumpc()
    {
        char_type ch;

        bool success = xsgetn(&ch, sizeof(ch)) == sizeof(ch);

        return success ? traits_type::to_int_type(ch) : traits_type::eof();
    }

    //int_type sgetc();

#ifdef FEATURE_IOS_SPEEKC
    int_type speekc();
#endif

    // NOTE: this deviates from spec in that it won't wait for CR, for example,
    // to reflect characters are available
    //streamsize in_avail();
};

template<class TChar, class TStream, class Traits = ::std::char_traits <TChar> >
class native_streambuf : public streambuf<
        impl::native_streambuf<TChar, TStream, Traits> >
{
    typedef streambuf<
    impl::native_streambuf<TChar, TStream, Traits> > base_type;
public:
    native_streambuf(TStream& stream) : base_type(stream) {}
};

}

// traditional basic_streambuf, complete with virtual functions
template<class TChar, class Traits = std::char_traits<TChar>>
struct basic_streambuf : internal::streambuf<estd::internal::impl::basic_streambuf<TChar, Traits> >
{
    typedef internal::streambuf<estd::internal::impl::basic_streambuf<TChar, Traits> > base_type;
};

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
}
#else
}
/*
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
*/
#include "port/streambuf.h"
#endif


