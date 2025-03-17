#pragma once

#include "internal/iomanip.h"
#include "iosfwd.h"

namespace estd {

template <class TChar>
ESTD_CPP_CONSTEXPR_RET internal::setfill<TChar> setfill(TChar c)
{
    return internal::setfill<TChar>(c);
}


ESTD_CPP_CONSTEXPR_RET internal::setw setw(unsigned width)
{
    return internal::setw(width);
}


template <class TStreambuf, class TBase>
estd::detail::basic_ostream<TStreambuf, TBase>& operator <<(
    estd::detail::basic_ostream<TStreambuf, TBase>& out, internal::setfill<typename TStreambuf::char_type> sf)
{
    out.fill(sf.c);
    return out;
}

template <class TStreambuf, class TBase>
estd::detail::basic_ostream<TStreambuf, TBase>& operator <<(
    estd::detail::basic_ostream<TStreambuf, TBase>& out, internal::setw width)
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
        return base == 8 ? ios_base::oct :
            base == 10 ? ios_base::dec :
            base == 16 ? ios_base::hex :
            ios_base::fmtflags(0);
    }

    constexpr explicit setbase(int base) : fmt_{to_fmt(base)} {}

    template <class Streambuf, class Base>
    void operator()(detail::basic_ostream<Streambuf, Base>& out) const
    {
        out.setf(fmt_, ios_base::basefield);
    }
};


}
