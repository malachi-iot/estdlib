#pragma once

// just for IDE really - in production code, this file is always included by estd/ios.h
//#include "../ios.h"
#include "../type_traits.h"
#include "../locale.h"
#include <stdint.h>

namespace estd {

// C++ spec actually defines streamsize as signed to accomodate some streambuf operations
// which we don't support, so I'm gonna make them unsigned
typedef uint16_t streamoff;
typedef uint16_t streamsize;

class ios_base
{
public:
    typedef uint8_t fmtflags;

    static CONSTEXPR fmtflags dec = 0x01;
    static CONSTEXPR fmtflags hex = 0x02;
    static CONSTEXPR fmtflags basefield = dec | hex;

    static CONSTEXPR fmtflags left = 0x08;
    static CONSTEXPR fmtflags right = 0x10;
    static CONSTEXPR fmtflags adjustfield = left | right;

    static CONSTEXPR fmtflags unitbuf = 0x20;


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

    typedef uint8_t seekdir;

    static CONSTEXPR seekdir beg = 0x00;
    static CONSTEXPR seekdir end = 0x01;
    static CONSTEXPR seekdir cur = 0x02;

private:
    fmtflags fmtfl;
    iostate _iostate;

protected:
    static CONSTEXPR openmode _openmode_null = 0; // proprietary, default of 'text'

public:
    ios_base() : fmtfl(dec), _iostate(goodbit) {}

    fmtflags setf(fmtflags flags)
    { fmtflags prior = fmtfl; fmtfl |= flags; return prior; }

    fmtflags unsetf(fmtflags flags)
    { fmtflags prior = fmtfl; fmtfl &= ~flags; return prior; }

    fmtflags flags() const
    { return fmtfl; }

    fmtflags flags(fmtflags fmtfl)
    { fmtflags prior = fmtfl; this->fmtfl = fmtfl; return prior; }

    iostate rdstate() const
    { return _iostate; }

    void clear(iostate state = goodbit)
    { _iostate = state; }

    void setstate(iostate state)
    {
        _iostate |= state;
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
    bool is_unitbuf_set() const { return fmtfl & unitbuf; }

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

// eventually, depending on layering, we will use a pointer to a streambuf or an actual
// value of streambuf itself
template <class TStreambuf, bool use_pointer>
class basic_ios_base;

template <class TStreambuf>
class basic_ios_base<TStreambuf, true> : public ios_base
{
protected:
    typedef TStreambuf streambuf_type;
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
#else
    template <class TParam1>
    basic_ios_base(TParam1& p1) : _rdbuf(p1)
            {}
#endif
public:
    streambuf_type* rdbuf()
    { return &_rdbuf; }
};


//template<class TChar, class Traits = std::char_traits <TChar>>
template<class TStreambuf, bool use_pointer = false>
class basic_ios : public basic_ios_base<TStreambuf, use_pointer>
{
public:
    typedef basic_ios_base<TStreambuf, use_pointer> base_type;
    typedef typename base_type::streambuf_type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;

protected:
    basic_ios() {}

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    basic_ios(TArgs&&...args) : base_type(std::forward<TArgs>(args)...) {}
#else
    template <class TParam1>
    basic_ios(TParam1& p) : base_type(p) {}

    template <class TParam1>
    basic_ios(TParam1* p) : base_type(p) {}
#endif

public:
    // NOTE: spec calls for this actually in ios_base, but for now putting it
    // here so that it can reach into streambuf to grab it.  A slight but notable
    // deviation from standard C++
    experimental::locale getloc() const
    {
        experimental::locale l;
        return l;
    }

    char_type widen(char c) const
    {
        experimental::ctype<char_type> ctype;
        return ctype.widen(c);
    }

    char narrow(char_type c, char /* default */)
    {
        return c;
    }
};


}}
