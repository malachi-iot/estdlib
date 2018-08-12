#pragma once

// just for IDE really - in production code, this file is always included by estd/ios.h
#include "../ios.h"
#include "../type_traits.h"

namespace estd { namespace internal {

template <class TStreambuf, bool use_pointer>
class basic_ios_base;

template <class TStreambuf>
class basic_ios_base<TStreambuf, true> : public ios_base
{
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
    typedef TStreambuf streambuf_type;

    // currently (temporarily) hard wired to streambufs which know about a stream object
    // to interact with
    typedef typename streambuf_type::stream_type stream_type;

protected:
    streambuf_type _rdbuf;

    basic_ios_base(stream_type &stream) : _rdbuf(stream)
            {}

    template <class _TStream, class ...TArgs>
    basic_ios_base(_TStream& stream, TArgs...args) : _rdbuf(stream, args...) {}

    streambuf_type* rdbuf()
    { return &_rdbuf; }
};


//template<class TChar, class Traits = std::char_traits <TChar>>
template<class TStreambuf>
class basic_ios : public basic_ios_base<TStreambuf, false>
{
public:
    typedef typename TStreambuf::traits_type traits_type;
    typedef basic_ios_base<TStreambuf, false> base_type;

protected:
    template <class TParam1>
    basic_ios(TParam1& p) : base_type(p) {}

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