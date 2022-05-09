#pragma once

#include "internal/platform.h"
//#include "traits/char_traits.h"
#include "traits/allocator_traits.h" // for ESTD_FN_HAS_METHOD dependencies
#include "type_traits.h"
// FIX: Temporarily including this at the bottom to satisfy dependencies
//#include "port/streambuf.h"
#include "internal/impl/streambuf.h"
#include "internal/utility.h" // for ESTD_FN_HAS_METHOD itself

#include "internal/impl/streambuf/tags.h"
#include "internal/impl/streambuf/helpers.h"


//#include "features.h"
#include "locale.h"

namespace estd {

namespace internal {

// all the noop/defaults which we expect to hide/overload (not override specifically,
// since we're not making these virtual here)
struct streambuf_baseline
{

};

// TODO: TPolicy shall specify modes:
// 1. never blocking
// 2. blocking with timeout (including infinite timeout)
// 3. blocking
// Note that consuming istream/ostream may independently implement its own timeout code
// in which case 'never blocking' mode may be utilized for streambuf
template<class TImpl, class TPolicy = void>
class streambuf :
        public streambuf_baseline,
        public TImpl
{
    typedef TImpl base_type;
    typedef streambuf<TImpl, TPolicy> this_type;

public:
    typedef TImpl impl_type;
    typedef TPolicy policy_type;

    typedef typename TImpl::char_type char_type;
    typedef typename TImpl::traits_type traits_type;
    typedef typename traits_type::int_type int_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;
    typedef typename remove_const<char_type>::type nonconst_char_type;

    typedef internal::impl::experimental::streambuf_helper helper_type;

    friend helper_type;

    int_type sungetc()
    {
        return helper_type::sungetc(this);
    }

protected:

    // not yet used overflow helpers
    // overflow() without parameter looks loosely similar to sync, but
    // devoted to flushing output.  Spec verbiage indicates
    // actually spitting output to sink is optional, but implies heavily
    // that you'll be doing it.  Also the output to sink will be from the pending
    // put area, not specifically incoming ch
    // NOTE: We expect overflow to sometimes be a locus of timeout activity
    // consider a traits_type::pending() or similar to indicate a nonblocking
    // async activity (counterpart to traits_type::nodata() ).  That said,
    // maybe eof() can do that job albeit in a less informative way
    /*
    template <class T = base_type>
    typename enable_if<has_overflow_method<T>::value, int_type>::type
    overflow(int_type ch = traits_type::eof())
    {
        return base_type::overflow(ch);
    }

    template <class T = base_type>
    typename enable_if<!has_overflow_method<T>::value, int_type>::type
    overflow(int_type = traits_type::eof())
    {
        return traits_type::eof();
    } */

public:
#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    streambuf(TArgs&&...args) : 
        base_type(std::forward<TArgs>(args)...)
    {}
#else
    // some streambufs don't need any initialization at the base level
    streambuf() {}

    template <class Param1>
    streambuf(Param1& p1) : base_type(p1)
    {}
#endif

    /*
#ifdef FEATURE_CPP_MOVESEMANTIC
    streambuf(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif */

    // http://putka.upm.si/langref/cplusplus.com/reference/iostream/streambuf/sgetn/index.html
    // acts like many sbumpc calls
    streamsize sgetn(nonconst_char_type* s, streamsize count)
    {
        return this->xsgetn(s, count);
    }

    // NOTE: Deviation from spec, as xsputn itself is expected to do all the
    // overflow trickery
    streamsize sputn(const char_type *s, streamsize count)
    {
        streamsize written = this->xsputn(s, count);

        // TODO: Consider a non-function-present flavor of optimizing to know if
        // overflow is even implemented so as to optimize this portion
        if(written < count)
        {
            s += written;

            if(this->overflow(*s) != traits_type::eof())
            {
                // getting here means one more character was placed into put area
                count -= ++written;

                // a successful overflow indicates more put area
                // is available, so give it a shot
                written += this->xsputn(s + 1, count);
            }
        }

        return written;
    }

    int_type sbumpc()
    {
        return helper_type::sbumpc_evaporated(this);
    }

    int_type snextc()
    {
        int_type ch = this->sbumpc();

        if(ch == traits_type::eof())
            return traits_type::eof();
        else
            return this->sgetc();
    }

    int_type sgetc()
    {
        if(base_type::xin_avail() == 0)
            return base_type::underflow();

        int_type ch = traits_type::to_int_type(base_type::xsgetc());

        return ch;
    }

    // sgetc implies nonblocking, but in fact typically does block in std environments
    // speekc gauruntees nonblocking.  however, since we strive to make sgetc nonblocking
    // speekc might be considered superfluous and a deviation from spec, so for now it's
    // wrapped in a feature flag
#ifdef FEATURE_ESTD_IOS_SPEEKC
    int_type speekc();
#endif

    // NOTE: this deviates from spec in that it won't wait for CR, for example,
    // to reflect characters are available
    inline streamsize in_avail()
    {
        return base_type::showmanyc();
    }


    inline int pubsync()
    {
        return this->sync();
    }

    inline pos_type pubseekpos(int_type pos, ios_base::openmode which = ios_base::in | ios_base::out)
    {
        return this->seekpos(pos, which);
    }


    pos_type pubseekoff(off_type off, ios_base::seekdir way, ios_base::openmode which = ios_base::in | ios_base::out)
    {
        return this->seekoff(off, way, which);
    }


    // DEBT: Overflow is supposed to be protected, but our wrapper can't easily reach it
    // that way at the moment
    using base_type::overflow;

    // Underflow works pretty well as protected, but in our embedded+non-blocking
    // world, public access to this is desirable to avoid minor overhead of an sgetc call
    using base_type::underflow;
};

template<class TChar, class TStream, class Traits = ::estd::char_traits <TChar> >
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
template<class TChar, class Traits = estd::char_traits<TChar> >
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
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;

protected:
    // TODO: May have to turn this into an is-a relationship so that we can
    // get at its protected methods
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

    virtual int_type overflow(int_type ch = traits_type::eof()) OVERRIDE
    {
        return _rdbuf.overflow();
    }

public:
    template <class TParam1>
    basic_streambuf_wrapped(TParam1& p) : _rdbuf(p) {}

};

}}


#include "port/streambuf.h"
#include "exp/streambuf-traits.h"


