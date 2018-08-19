#pragma once

#include "internal/platform.h"
//#include "traits/char_traits.h"
#include "traits/allocator_traits.h" // for ESTD_HAS_METHOD_EXPERIMENTAL1 dependencies
#include "type_traits.h"
// FIX: Temporarily including this at the bottom to satisfy dependencies
//#include "port/streambuf.h"
//#include "internal/ios.h"
#include "internal/impl/streambuf.h"


//#include "features.h"
#include "locale.h"

namespace estd {

namespace internal {

// TODO: Replace regular ESTD_HAS_METHOD_EXPERIMENTAL later when we are ready to merge
// and ready to handle possible breakages.  Working pretty well, just about ready to
// rename it to ESTD_HAS_METHOD
#define ESTD_HAS_METHOD_EXPERIMENTAL1(ret_type, method_name, ...) \
template <class T> struct has_##method_name##_method : has_member_base \
{ \
    template <typename C> static CONSTEXPR yes& test(reallyHas<ret_type (C::*)(__VA_ARGS__), &C::method_name>* /*unused*/) \
    { return yes_value; }  \
\
    template <typename> static CONSTEXPR no& test(...) { return no_value; } \
\
    static CONSTEXPR bool value = sizeof(test<T>(0)) == sizeof(yes); \
};

template<class TImpl>
class streambuf : public TImpl
{
    typedef TImpl base_type;

public:
    // estd::internal::impl::native_streambuf<TChar, TStream, Traits
    typedef typename TImpl::char_type char_type;
    typedef typename TImpl::traits_type traits_type;
    typedef typename traits_type::int_type int_type;

    ESTD_HAS_METHOD_EXPERIMENTAL1(int_type, sputc, char_type)
    ESTD_HAS_METHOD_EXPERIMENTAL1(int_type, sgetc)
    ESTD_HAS_METHOD_EXPERIMENTAL1(int_type, sbumpc)
    ESTD_HAS_METHOD_EXPERIMENTAL1(int, sync)
    ESTD_HAS_METHOD_EXPERIMENTAL1(char_type*, gptr)
    ESTD_HAS_METHOD_EXPERIMENTAL1(char_type*, egptr)
    ESTD_HAS_METHOD_EXPERIMENTAL1(char_type*, pptr)
    ESTD_HAS_METHOD_EXPERIMENTAL1(char_type*, epptr)
    ESTD_HAS_METHOD_EXPERIMENTAL1(int_type, underflow)

protected:

    /*
     * Not doing these because I bet polymorphism breaks if you do
    streamsize xsputn(const char_type *s, streamsize count)
    {
        return base_type::xsputn(s, count);
    }

    streamsize xsgetn(char_type *s, streamsize count)
    {
        return base_type::xsgetn(s, count);
    } */

public:
    // some streambufs don't need any initialization at the base level
    streambuf() {}


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
        return this->xsgetn(s, count);
    }

    streamsize sputn(const char_type *s, streamsize count)
    {
        return this->xsputn(s, count);
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
        bool success = sputn(&ch, sizeof(ch)) == sizeof(ch);
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

        bool success = sgetn(&ch, sizeof(ch)) == sizeof(ch);

        return success ? traits_type::to_int_type(ch) : traits_type::eof();
    }

    // TODO: sgetc is actually more of a wrapper around underflow, who
    // mainly interacts with buffers otherwise so consider implementing
    // underflow instead for our low level character acquisition.  Note though,
    // clumsily, underflow is technically also responsible for then
    // producing (not just populating) a new gptr with
    // data - if any

    /*
     *  Reasonable code, but enabling this disables any existing sgetc
     *
    template <class T = base_type>
    typename enable_if<has_underflow_method<T>::value, int_type>::type
    sgetc()
    {
        return this->underflow();
    } */

    /* NOTE: implementation of sgetc is kind of specific, so we can't make
     * a generic handler like the others [reading a char without advancing]
    template <class T = base_type>
    typename enable_if<has_sgetc_method<T>::value, int_type>::type
    sgetc()
    {
        return base_type::sgetc();
    }

    template <class T = base_type>
    typename enable_if<!has_sgetc_method<T>::value, int_type>::type
    sgetc()
    {
        return -1;
    } */

    // sgetc implies nonblocking, but in fact typically does block in std environments
    // speekc gauruntees nonblocking.  however, since we strive to make sgetc nonblocking
    // speekc might be considered superfluous and a deviation from spec, so for now it's
    // wrapped in a feature flag
#ifdef FEATURE_ESTD_IOS_SPEEKC
    int_type speekc();
#endif

    // NOTE: this deviates from spec in that it won't wait for CR, for example,
    // to reflect characters are available
    //streamsize in_avail();

    template <class T = base_type>
    typename enable_if<!has_sync_method<T>::value, int>::type
    pubsync()
    {
        return 0;
    }

    template <class T = base_type>
    typename enable_if<has_sync_method<T>::value, int>::type
    pubsync()
    {
        return this->sync();
    }
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
template<class TChar, class Traits = std::char_traits<TChar> >
struct basic_streambuf : internal::streambuf<estd::internal::impl::basic_streambuf<TChar, Traits> >
{
    typedef internal::streambuf<estd::internal::impl::basic_streambuf<TChar, Traits> > base_type;
};

namespace internal {
// Could use a better name
// more or less turns a non-virtualized streambuf into a virtualized one
// but also could be used to wrap a virtualized one
template<class TStreambuf>
struct basic_streambuf_wrapped :
        basic_streambuf<
                typename estd::remove_reference<TStreambuf>::type::char_type,
                typename estd::remove_reference<TStreambuf>::type::traits_type
        >
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::char_type char_type;

private:
    TStreambuf _rdbuf;

    virtual streamsize xsgetn(char_type* s, streamsize count) OVERRIDE
    {
        return _rdbuf.xsgetn(s, count);
    }

    virtual streamsize xsputn(const char_type* s, streamsize count) OVERRIDE
    {
        return _rdbuf.xsputn(s, count);
    }

    virtual int sync() OVERRIDE
    {
        return _rdbuf.pubsync();
    }

public:
    template <class TParam1>
    basic_streambuf_wrapped(TParam1& p) : _rdbuf(p) {}

};

}}


#include "port/streambuf.h"



