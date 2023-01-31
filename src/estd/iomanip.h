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


}
