#pragma once

#include "../type_traits.h"

namespace estd { namespace experimental {

template <class TStreambuf>
struct streambuf_traits
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::char_type char_type;
    typedef typename estd::span<char_type> span_type;

    static span_type gdata(streambuf_type& s)
    {
        return span_type(s.gptr(), s.in_avail());
    }


    static span_type pdata(streambuf_type& s)
    {
        return span_type(s.pptr(), s.epptr() - s.pptr());
    }
};

}}