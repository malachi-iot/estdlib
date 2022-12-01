#pragma once

#include "../../../streambuf.h"
#include "../../../internal/impl/streambuf/tags.h"

#include <pico/stdio/driver.h>

namespace estd {

namespace internal { namespace impl {

// DEBT: Use evaporators here

template <stdio_driver_t* d = nullptr>
class pico_stdio_streambuf_provider2;


// NOTE: layer0 is nifty and viable, but when used within ostream it doesn't
// save much since streambuf itself we don't attempt to evaporate yet (we
// could, just haven't optimized that far)
namespace layer0 {

template <stdio_driver_t* d>
class pico_stdio_streambuf_provider
{
protected:
    static constexpr stdio_driver_t* value() { return d; }
};

// Would prefer this if we can
template <stdio_driver_t& d>
class pico_stdio_streambuf_provider_exp
{
protected:
    static constexpr stdio_driver_t* value() { return &d; }
};


}

class pico_stdio_streambuf_provider
{
protected:
    stdio_driver_t* d;

    stdio_driver_t* value() const { return d; }
};


template <stdio_driver_t* d>
class pico_stdio_streambuf_provider2 : public layer0::pico_stdio_streambuf_provider<d>
{

};

template <>
class pico_stdio_streambuf_provider2<nullptr> : public pico_stdio_streambuf_provider
{
protected:
    pico_stdio_streambuf_provider2(stdio_driver_t* d)
    {
        this->d = d;
    }
};



template <class TTraits, class TProvider>
class pico_stdio_streambuf : public streambuf_base<TTraits>,
    TProvider
{
    typedef streambuf_base<TTraits> base_type;
    typedef TProvider provider;

protected:
    typedef TTraits traits_type;

    using typename base_type::int_type;
    using typename base_type::char_type;

    streamsize xsputn(const char_type* s, streamsize count)
    {
        provider::value()->out_chars(s, count);
        return count;
    }

    // EXPERIMENTAL, UNTESTED
    streamsize xsgetn(char_type* s, streamsize count)
    {
        int rc = provider::value()->in_chars(&s, count);

        if(rc < PICO_OK) return 0;

        return rc;
    }

public:
    pico_stdio_streambuf() = default;
    pico_stdio_streambuf(stdio_driver_t* d) : provider(d) {}

    //template <class ...TArgs>
    //pico_stdio_streambuf(TArgs...args) : provider(std::forward<TArgs>(args)...) {}

    // EXPERIMENTAL, UNTESTED    
    int_type sbumpc()
    {
        char c;

        int rc = provider::value()->in_chars(&c, 1);

        if(rc == PICO_ERROR_NO_DATA)
            return traits_type::eof();
        else
            return traits_type::to_int_type(c);
    }

    int_type sputc(char_type ch)
    {
        provider::value()->out_chars(&ch, 1);
        return traits_type::to_int_type(ch);
    }
};


}}

namespace layer0 {

template <class TChar, stdio_driver_t* d, class TTraits = estd::char_traits<TChar> >
using basic_pico_stdio_streambuf = estd::internal::streambuf<
        internal::impl::pico_stdio_streambuf<TTraits,
            internal::impl::layer0::pico_stdio_streambuf_provider<d> > >;

}

template <class TChar, class TTraits = estd::char_traits<TChar> >
using basic_pico_stdio_streambuf = estd::internal::streambuf<
        internal::impl::pico_stdio_streambuf<TTraits,
            internal::impl::pico_stdio_streambuf_provider > >;

template <class TChar, stdio_driver_t* d, class TTraits = estd::char_traits<TChar> >
using basic_pico_stdio_streambuf2 = estd::internal::streambuf<
        internal::impl::pico_stdio_streambuf<TTraits,
            internal::impl::pico_stdio_streambuf_provider2<d> > >;

}
