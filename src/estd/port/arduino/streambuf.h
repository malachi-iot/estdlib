#pragma once

#include <Arduino.h>
#include "../../streambuf.h"

namespace estd {

namespace internal { namespace impl {

template <class TTraits>
class arduino_streambuf : public streambuf_base<TTraits>
{
    typedef streambuf_base<TTraits> base_type;

    Print* print;

public:
    typedef typename base_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;

    arduino_streambuf(Print* print) : print(print) {}

    streamsize xsputn(const char_type* s, streamsize count)
    {
        return print->write(s, count);
    }

    int_type sputc(char_type ch)
    {
        if(print->write(ch))
            return traits_type::to_int_type(ch);
        else
            return traits_type::eof();
    }
};

}}

typedef estd::internal::streambuf<
    internal::impl::arduino_streambuf<
        estd::char_traits<char> > > arduino_streambuf;

}