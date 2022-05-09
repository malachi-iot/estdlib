#pragma once

// just for IDE really - in production code, this file is always included by estd/ios.h
//#include "../ios.h"
#include "../type_traits.h"
#include "locale.h"
#include <stdint.h>
#include "../iosfwd.h"
#include "../variant.h"
#include "../thread.h"
#include "ios_policy.h"

namespace estd {

#ifndef FEATURE_ESTD_AGGRESIVE_BITFIELD
#define FEATURE_ESTD_AGGRESIVE_BITFIELD 1
#endif




class ios_base
{
public:
    typedef uint8_t fmtflags;

    // NOTE: Spec appears to conflict with itself.
    // https://en.cppreference.com/w/cpp/io/ios_base/fmtflags suggests we have carte blanche
    // do make these any values we wish, but
    // https://en.cppreference.com/w/cpp/locale/num_get/get strongly implies that 'dec'
    // is expected to be zero
    static CONSTEXPR fmtflags dec = 0x01;
    static CONSTEXPR fmtflags hex = 0x02;
    static CONSTEXPR fmtflags oct = 0x03;
    static CONSTEXPR fmtflags basefield = dec | hex;

    static CONSTEXPR fmtflags left = 0x08;
    static CONSTEXPR fmtflags right = 0x10;
    static CONSTEXPR fmtflags adjustfield = left | right;

    static CONSTEXPR fmtflags boolalpha = 0x20;
    static CONSTEXPR fmtflags unitbuf = 0x40;


    typedef uint8_t openmode;

    static CONSTEXPR openmode app = 0x01;
    static CONSTEXPR openmode binary = 0x02;
    static CONSTEXPR openmode in = 0x04;
    static CONSTEXPR openmode out = 0x08;

    typedef uint8_t iostate;

    static CONSTEXPR iostate goodbit = 0x00;
    static CONSTEXPR iostate badbit = 0x01;
    static CONSTEXPR iostate failbit = 0x02;
    static CONSTEXPR iostate eofbit = 0x04;

    // Non standard, experimental.  Reflects that we are in a wait state
    // to see if there is any more data.  Maps to 'showmanyc' value of 0.
    static CONSTEXPR iostate nodatabit = 0x08;

    typedef uint8_t seekdir;

    static CONSTEXPR seekdir beg = 0x00;
    static CONSTEXPR seekdir end = 0x01;
    static CONSTEXPR seekdir cur = 0x02;

private:
    struct
    {
#if FEATURE_ESTD_AGGRESIVE_BITFIELD
        fmtflags fmtfl_ : 8;
        iostate iostate_ : 4;
#else
        fmtflags fmtfl_;
        iostate iostate_;
#endif

    }   state_;

protected:
    static CONSTEXPR openmode _openmode_null = 0; // proprietary, default of 'text'

    // remove state, not official call
    // UNTESTED
    void unsetstate(iostate state)
    {
        state_.iostate_ &= ~state;
    }

public:
    // DEBT: Use initializer lists for compilers that have it
    //ios_base() : fmtfl(dec), _iostate(goodbit) {}
    ios_base()
    {
        state_.fmtfl_ = dec;
        state_.iostate_ = goodbit;
    }

    fmtflags setf(fmtflags flags)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ |= flags; return prior; }

    fmtflags setf(fmtflags flags, fmtflags mask)
    {
        fmtflags prior = state_.fmtfl_;
        state_.fmtfl_ &= ~mask;
        state_.fmtfl_ |= flags;
        return prior;
    }

    fmtflags unsetf(fmtflags flags)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ &= ~flags; return prior; }

    fmtflags flags() const
    { return state_.fmtfl_; }

    fmtflags flags(fmtflags fmtfl)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ = fmtfl; return prior; }

    iostate rdstate() const
    { return state_.iostate_; }

    void clear(iostate state = goodbit)
    { state_.iostate_ = state; }

    void setstate(iostate state)
    {
        state_.iostate_ |= state;
    }

    bool good() const
    { return rdstate() == goodbit; }

    bool bad() const
    { return rdstate() & badbit; }

    bool fail() const
    { return rdstate() & failbit || rdstate() & badbit; }

    bool eof() const
    { return rdstate() & eofbit; }

protected:
    // internal call which we may make a layer0 version for optimization
    bool is_unitbuf_set() const { return state_.fmtfl_ & unitbuf; }

};

// NOTE: these are not heeded quite yet
inline ios_base& unitbuf(ios_base& s)
{
    s.setf(ios_base::unitbuf);
    return s;
}

inline ios_base& nounitbuf(ios_base& s)
{
    s.unsetf(ios_base::unitbuf);
    return s;
}


namespace experimental {

// 21MAR21 TODO: Revisit this and combine with new ios_base_policy
struct ios_policy
{
    // whether to do timeouts at all
    // for now timeout parameters apply equally to both istream and ostream
    typedef void do_timeout_tag;

    // whether to do yield/sleeps while polling
    typedef void do_sleep_tag;

    // doing a timeout also means that we attempt retries on read/write operations
    static CONSTEXPR bool do_timeout() { return true; }

    static CONSTEXPR size_t timeout_in_ms() { return 1000; }

    static CONSTEXPR size_t sleep_in_ms() { return 1000; }
};

template <typename>
struct has_typedef { typedef void type; };

template<typename T, typename = void>
struct is_do_timeout_tag_present : estd::false_type {};

template<typename T>
struct is_do_timeout_tag_present<T, typename has_typedef<typename T::do_timeout_tag>::type> : estd::true_type {};


}

namespace internal {

// Copy/pasted from bitness deducer
template<bool>
struct Range;

template <class TStreambuf, estd::experimental::istream_flags::flag_type>
struct ios_blocking_policy;

template <class TStreambuf>
struct ios_blocking_policy<TStreambuf, estd::experimental::istream_flags::non_blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename streambuf_type::int_type int_type;

    struct blocking_type
    {
        static int_type sgetc(const ios_base&, streambuf_type* rdbuf)
        {
            return rdbuf->sgetc();
        }

        static void on_nodata(ios_base& in, streambuf_type* rdbuf, unsigned sz)
        {
            in.setstate(ios_base::nodatabit);
            // EXPERIMENTAL
            // Back off the characters when in nonblocking mode so that one may attempt again
            // DEBT: Consider doing a gbump as an optimization, remembering that it does no
            // underflow checks
            for(unsigned i = sz; i > 0; --i)
                rdbuf->sungetc();
        }
    };
};

template <class TStreambuf>
struct ios_blocking_policy<TStreambuf, estd::experimental::istream_flags::blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename streambuf_type::int_type int_type;

    struct blocking_type
    {
        static void on_nodata(ios_base& in, streambuf_type* rdbuf, unsigned sz) {}

        static int_type sgetc(const ios_base&, streambuf_type* rdbuf)
        {
            for(;;)
            {
                // DEBT: Need timeout logic here
                estd::streamsize avail = rdbuf->in_avail();

                if(avail == -1) return traits_type::eof();

                if(avail == 0)
                {
                    estd::this_thread::yield();
                }
                else
                {
                    return rdbuf->sgetc();
                }
            }
        }
    };
};

template <class TStreambuf>
struct ios_blocking_policy<TStreambuf, estd::experimental::istream_flags::runtime_blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::int_type int_type;
    typedef ios_blocking_policy<TStreambuf, estd::experimental::istream_flags::non_blocking> nonblocking_policy;
    typedef ios_blocking_policy<TStreambuf, estd::experimental::istream_flags::blocking> blocking_policy;

    // FIX: Need to interrogate 'in' to determine whether we're blocking or not blocking
    struct blocking_type
    {
        static void on_nodata(ios_base& in, streambuf_type* rdbuf, unsigned sz)
        {
            blocking_policy::on_nodata(in, rdbuf, sz);
        }

        static int_type sgetc(const ios_base& in, streambuf_type* rdbuf)
        {
            return blocking_policy::sgetc(in, rdbuf);
        }
    };
};


template <class TStreambuf,
    estd::experimental::istream_flags::flag_type flags =
        estd::experimental::istream_flags::blocking> // FIX: Temporarily set to blocking as we code out feature
        //estd::experimental::istream_flags::_default>
struct ios_base_policy : 
    ios_blocking_policy<TStreambuf, flags & estd::experimental::istream_flags::block_mask>
{
    typedef experimental::locale locale_type;

    static CONSTEXPR estd::experimental::istream_flags::flag_type blocking()
    {
        return flags & estd::experimental::istream_flags::block_mask;
    }
};

// eventually, depending on layering, we will use a pointer to a streambuf or an actual
// value of streambuf itself
template <class TStreambuf, bool use_pointer>
class basic_ios_base;

template <class TStreambuf>
class basic_ios_base<TStreambuf, true> : public ios_base
{
protected:
    typedef typename remove_reference<TStreambuf>::type streambuf_type;
    streambuf_type* _rdbuf;

    streambuf_type* rdbuf() const { return _rdbuf; }

protected:
    basic_ios_base(streambuf_type* sb) : _rdbuf(sb) {}

    void init(streambuf_type* sb)
    {
        _rdbuf = sb;
    }

    streambuf_type* rdbuf(streambuf_type* sb)
    {
        clear();
        streambuf_type temp = _rdbuf;
        _rdbuf = sb;
        return temp;
    }
};


// this one assumes for now our special 'native_streambuf' which shall be the
// de-specialized version of our basic_streambuf
template <class TStreambuf>
class basic_ios_base<TStreambuf, false> : public ios_base
{
public:
    typedef typename remove_reference<TStreambuf>::type streambuf_type;

protected:
    TStreambuf _rdbuf;

    basic_ios_base() {}

    // TODO: constructor needs cleanup here

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    basic_ios_base(TArgs&&...args) : 
        _rdbuf(std::forward<TArgs>(args)...) {}

    basic_ios_base(streambuf_type&& streambuf) :
        _rdbuf(std::move(streambuf))    {}
#endif
    basic_ios_base(streambuf_type& streambuf) :
        _rdbuf(streambuf) {}

    template <class TParam1>
    basic_ios_base(TParam1& p1) : _rdbuf(p1)
            {}

public:
    streambuf_type* rdbuf()
    { return &_rdbuf; }
};


//template<class TChar, class Traits = std::char_traits <TChar>>
template<class TStreambuf, bool use_pointer = false,
    class TPolicy = ios_base_policy<TStreambuf> >
class basic_ios : public basic_ios_base<TStreambuf, use_pointer>,
    estd::internal::struct_evaporator<TPolicy>
{
public:
    typedef basic_ios_base<TStreambuf, use_pointer> base_type;
    typedef typename base_type::streambuf_type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;

    typedef TPolicy policy_type;
    typedef typename policy_type::locale_type locale_type;
    typedef typename policy_type::blocking_type blocking_type;

    typedef typename estd::internal::struct_evaporator<TPolicy> policy_provider_type;
    typedef typename policy_provider_type::evaporated_type evaporated_policy_type;

protected:
    basic_ios() {}

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    basic_ios(TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...) {}

    basic_ios(streambuf_type&& streambuf) :
        base_type(std::move(streambuf)) {}
#else
    template <class TParam1>
    basic_ios(TParam1& p) : base_type(p) {}

    template <class TParam1>
    basic_ios(TParam1* p) : base_type(p) {}
#endif
    basic_ios(streambuf_type& streambuf) :
        base_type(streambuf) {}

public:
    evaporated_policy_type policy() const { return policy_provider_type::value(); }
    
    // NOTE: spec calls for this actually in ios_base, but for now putting it
    // here so that it can reach into streambuf to grab it.  A slight but notable
    // deviation from standard C++
    locale_type getloc() const
    {
        locale_type l;
        return l;
    }

    char_type widen(char c) const
    {
        return experimental::use_facet<experimental::ctype<char_type> >(getloc()).widen(c);
        experimental::ctype<char_type> ctype;
        return ctype.widen(c);
    }

    char narrow(char_type c, char /* default */)
    {
        return c;
    }
};


}}
