#pragma once

#include <Arduino.h>
#include "../../streambuf.h"
#include "../../internal/impl/streambuf/tags.h"

namespace estd {

namespace internal { namespace impl {

template <class TTraits, class TStream>
class arduino_streambuf_base : public streambuf_base<TTraits>
{
protected:
    typedef TStream stream_type;

    TStream* const print;

    arduino_streambuf_base(TStream* print) : print(print) {}
};

// This lets you do things with just a raw 'Print' type
template <class TTraits, class TBase = arduino_streambuf_base<TTraits, Print> >
class arduino_ostreambuf : public TBase 
{
    typedef TBase base_type;

public:
    typedef typename base_type::traits_type traits_type;
    typedef typename base_type::stream_type stream_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;

    arduino_ostreambuf(stream_type& print) : base_type(&print) {}

    Print& underlying() const { return *this->print; }

    streamsize xsputn(const char_type* s, streamsize count)
    {
        return underlying().write((const uint8_t*)s, count);
    }

    int_type sputc(char_type ch)
    {
        if(underlying().write(ch))
            return traits_type::to_int_type(ch);
        else
            return traits_type::eof();
    }
};

// To do input, we are able to include output stream smarts for zero cost.
// Therefore, there is no need for arduino_istreambuf.  We can combine into
// a std-natural normal i/o streambuf
template <class TTraits>
class arduino_streambuf : public arduino_ostreambuf<TTraits, 
    arduino_streambuf_base<TTraits, Stream> >,
    estd::experimental::streambuf_sbumpc_tag
{
    typedef arduino_ostreambuf<TTraits, arduino_streambuf_base<TTraits, Stream> > base_type;

public:
    typedef typename base_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;

    arduino_streambuf(Stream& stream) : base_type(stream) {}

    Stream& underlying() const { return *this->print; }

    // NOTE: Not active yet
    int_type sbumpc()
    {
        int_type ch = underlying().read();
        return ch == -1 ? traits_type::eof() : traits_type::to_char_type(ch);
    }

    int xin_avail() const
    {
        return underlying().available();
    }

    int_type underflow() const
    {
        int_type ch = underlying().peek();
        return ch == -1 ? traits_type::eof() : traits_type::to_char_type(ch);
    }

    int_type uflow() const
    {
        int_type ch = underlying().read();
        return ch == -1 ? traits_type::eof() : traits_type::to_char_type(ch);
    }

    streamsize xsgetn(char_type* s, streamsize count)
    {
        return underlying().readBytes(s, count);
    }

    char_type xsgetc() const
    {
        int_type ch = underlying().peek();
        // DEBT: Caller 'sgetc' checks xin_avail() and underflow() to
        // pretty much gauruntee we never get to a ch == -1 here
        return ch == -1 ? traits_type::eof() : traits_type::to_char_type(ch);
    }

    void gbump(int count)
    {
        while(count--) underlying().read();
    }
};

}}

typedef estd::internal::streambuf<
    internal::impl::arduino_ostreambuf<
        estd::char_traits<char> > > arduino_ostreambuf;

typedef estd::internal::streambuf<
    internal::impl::arduino_streambuf<
        estd::char_traits<char> > > arduino_streambuf;

}