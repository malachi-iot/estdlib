#pragma once

#include "fwd/streambuf.h"
#include "iosfwd.h"
#include "impl/streambuf.h"
#include "impl/streambuf/tags.h"
#include "type_traits.h"


namespace estd { namespace internal {

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

// DEBT: Belongs elsewhere
// DEBT: Making 'internal' and not 'layer1' because I can't figure out a good name for this guy yet
#if __cpp_alias_templates
template <class Streambuf, unsigned N = 64>
using out_buffered_stringbuf = detail::streambuf<impl::out_buffered_stringbuf<Streambuf, N> >;
#endif

}}