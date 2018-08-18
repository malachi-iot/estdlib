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
    static CONSTEXPR fmtflags unitbuf = 0x04;
    static CONSTEXPR fmtflags basefield = dec | hex;

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

private:
    fmtflags fmtfl;
    iostate _iostate;

protected:
    static CONSTEXPR openmode _openmode_null = 0; // proprietary, default of 'text'

public:
    ios_base() : fmtfl(dec), _iostate(goodbit) {}

    fmtflags flags() const
    { return fmtfl; }

    fmtflags flags(fmtflags fmtfl)
    { return this->fmtfl = fmtfl; }

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
};


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

    // currently (temporarily) hard wired to streambufs which know about a stream object
    // to interact with
    //typedef typename streambuf_type::stream_type stream_type;

protected:
    TStreambuf _rdbuf;

    basic_ios_base() {}

    template <class TParam1>
    basic_ios_base(TParam1& p1) : _rdbuf(p1)
    //basic_ios_base(stream_type &stream) : _rdbuf(stream)
            {}

    template <class _TStream, class ...TArgs>
    basic_ios_base(_TStream& stream, TArgs...args) : _rdbuf(stream, args...) {}

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

    template <class TParam1>
    basic_ios(TParam1& p) : base_type(p) {}

    template <class TParam1>
    basic_ios(TParam1* p) : base_type(p) {}

public:
    // NOTE: spec calls for this actually in ios_base, but for now putting it
    // here so that it can reach into streambuf to grab it.  A slight but notable
    // deviation from standard C++
    experimental::locale getloc() const
    {
        experimental::locale l;
        return l;
    }
};


}}
