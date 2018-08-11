#pragma once

// just for IDE really - in production code, this file is always included by estd/ios.h
#include "../ios.h"

namespace estd { namespace internal {

//template<class TChar, class Traits = std::char_traits <TChar>>
template<class TStreambuf>
class basic_ios : public ios_base
{
public:
    //typedef basic_streambuf <TChar, Traits> basic_streambuf_t;
    //typedef Traits traits_type;
    typedef TStreambuf basic_streambuf_t;
    typedef typename TStreambuf::traits_type traits_type;

protected:
#ifdef FEATURE_IOS_STREAMBUF_FULL
    basic_streambuf_t* _rdbuf;

public:
    basic_streambuf_t* rdbuf() const { return _rdbuf; }
    basic_streambuf_t* rdbuf(basic_streambuf_t* sb)
    {
        clear();
        auto temp = _rdbuf;
        _rdbuf = sb;
        return temp;
    }

#else
    basic_streambuf_t _rdbuf;

    typedef typename basic_streambuf_t::stream_type stream_type;

    basic_ios(stream_type &stream) : _rdbuf(stream)
    {}

    template <class _TStream, class ...TArgs>
    basic_ios(_TStream& stream, TArgs...args) : _rdbuf(stream, args...) {}

public:
    basic_streambuf_t *rdbuf() const
    { return (basic_streambuf_t *) &_rdbuf; }

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
};


}}