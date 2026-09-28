#pragma once

#include "internal/iomanip.h"
#include "internal/ios_base.h"
#include "iosfwd.h"

namespace estd {

template <class Char>
constexpr internal::setfill<Char> setfill(Char c)
{
    return internal::setfill<Char>(c);
}


constexpr internal::setw setw(unsigned width)
{
    return internal::setw(width);
}


template <class Streambuf, class Base>
detail::basic_ostream<Streambuf, Base>& operator <<(
    detail::basic_ostream<Streambuf, Base>& out,
    internal::setfill<typename Streambuf::char_type> sf)
{
    out.fill(sf.c);
    return out;
}

template <class Streambuf, class Base>
detail::basic_ostream<Streambuf, Base>& operator <<(
    detail::basic_ostream<Streambuf, Base>& out, internal::setw width)
{
    out.width(width.width);
    return out;
}

class setbase : public detail::ostream_functor_tag
{
    const ios_base::fmtflags fmt_;

public:
    static constexpr ios_base::fmtflags to_fmt(int base)
    {
        return
#if FEATURE_ESTD_OSTREAM_OCTAL
            base == 8 ? ios_base::oct :
#endif
            base == 10 ? ios_base::dec :
            base == 16 ? ios_base::hex :
            ios_base::fmtflags{};
    }

    constexpr explicit setbase(int base) : fmt_{to_fmt(base)} {}

    template <class Streambuf, class Base>
    void operator()(detail::basic_ostream<Streambuf, Base>& out) const
    {
        out.setf(fmt_, ios_base::basefield);
    }
};


}
