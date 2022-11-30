#pragma once

#include "../../../streambuf.h"
#include "../../../internal/impl/streambuf/tags.h"

#include <pico/stdio/driver.h>

namespace estd {

namespace internal { namespace impl {

template <class TTraits, stdio_driver_t* d>
class pico_streambuf : public streambuf_base<TTraits>
{
    typedef streambuf_base<TTraits> base_type;

protected:
    typedef TTraits traits_type;

    using typename base_type::int_type;
    using typename base_type::char_type;

    streamsize xsputn(const char_type* s, streamsize count)
    {
        d->out_chars(s, count);
        return count;
    }

    // EXPERIMENTAL, UNTESTED
    streamsize xsgetn(char_type* s, streamsize count)
    {
        int rc = d->in_chars(&s, count);

        if(rc < PICO_OK) return 0;

        return rc;
    }

public:

    // EXPERIMENTAL, UNTESTED    
    int_type sbumpc()
    {
        char c;

        int rc = d->in_chars(&c, 1);

        if(rc == PICO_ERROR_NO_DATA)
            return traits_type::eof();
        else
            return traits_type::to_int_type(c);
    }

    int_type sputc(char_type ch)
    {
        d->out_chars(&ch, 1);
        return traits_type::to_int_type(ch);
    }
};


}}

template <class TChar, stdio_driver_t* d, class TTraits = estd::char_traits<TChar> >
using basic_pico_streambuf = estd::internal::streambuf<
        internal::impl::pico_streambuf<TTraits, d> >;

//typedef basic_pico_streambuf<char> = pico_streambuf;

}
