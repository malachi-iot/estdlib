#pragma once

#include "internal/platform.h"
#include "internal/streambuf.h"
#include "traits/allocator_traits.h" // for ESTD_FN_HAS_METHOD dependencies
#include "type_traits.h"
// FIX: Temporarily including this at the bottom to satisfy dependencies
//#include "port/streambuf.h"
#include "internal/utility.h" // for ESTD_FN_HAS_METHOD itself

#include "internal/impl/streambuf/helpers.h"



//#include "features.h"
#include "locale.h"

namespace estd {

namespace internal {

// TODO: TPolicy shall specify modes:
// 1. never blocking
// 2. blocking with timeout (including infinite timeout)
// 3. blocking
// Note that consuming istream/ostream may independently implement its own timeout code
// in which case 'never blocking' mode may be utilized for streambuf
template<class Impl, class Policy>
class streambuf :
        public streambuf_baseline,
        public Impl
{
    typedef Impl base_type;
    typedef streambuf<Impl, Policy> this_type;

public:
    typedef Impl impl_type;
    typedef Policy policy_type;

    typedef typename Impl::char_type char_type;
    typedef typename Impl::traits_type traits_type;
    typedef typename traits_type::int_type int_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;
    typedef typename remove_const<char_type>::type nonconst_char_type;

    typedef internal::impl::streambuf_helper helper_type;

    // DEBT: 'friend helper_type' makes CLang in c++03 mode mad
    friend struct internal::impl::streambuf_helper;

#if __cpp_concepts
    static consteval void verify_istream()
    {
        static_assert(concepts::v1::impl::InStreambuf<this_type>, "Must conform to istreambuf concept");
    }

    static consteval void verify_ostream()
    {
        static_assert(concepts::v1::impl::OutStreambuf<this_type>, "Must conform to ostreambuf concept");
    }
#else
    static constexpr bool verify_istream() { return {}; }
    static constexpr bool verify_ostream() { return {}; }
#endif

    int_type sungetc()
    {
        return helper_type::sungetc(this);
    }

    ESTD_CPP_CONSTEXPR_RET int_type xsgetc() const
    {
        verify_istream();
        return traits_type::to_int_type(base_type::xsgetc());
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
    ESTD_CPP_FORWARDING_CTOR(streambuf)

    /*
#ifdef FEATURE_CPP_MOVESEMANTIC
    streambuf(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif */

    // http://putka.upm.si/langref/cplusplus.com/reference/iostream/streambuf/sgetn/index.html
    // acts like many sbumpc calls
    streamsize sgetn(nonconst_char_type* s, streamsize count)
    {
        verify_istream();
        return this->xsgetn(s, count);
    }

    // NOTE: Deviation from spec, as xsputn itself is expected to do all the
    // overflow trickery
    streamsize sputn(const char_type *s, streamsize count)
    {
        verify_ostream();

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

    int_type sgetc()
    {
        return base_type::xin_avail() == 0 ? underflow() : xsgetc();
    }

    ESTD_CPP_CONSTEXPR_RET int_type sgetc() const
    {
        return base_type::xin_avail() == 0 ? underflow() : xsgetc();
    }

    int_type snextc()
    {
        const int_type ch = sbumpc();

        return ch == traits_type::eof() ? traits_type::eof() : sgetc();
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
    streamsize in_avail()
    {
        return base_type::showmanyc();
    }

#if __cpp_constexpr
    constexpr streamsize in_avail() const
    {
        return base_type::showmanyc();
    }
#endif

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
template<class TChar, class Traits>
struct basic_streambuf : internal::streambuf<estd::internal::impl::basic_streambuf<TChar, Traits> >
{
    typedef internal::streambuf<estd::internal::impl::basic_streambuf<TChar, Traits> > base_type;
};

}


#include "port/streambuf.h"
#include "exp/streambuf-traits.h"


