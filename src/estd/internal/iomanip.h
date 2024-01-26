#pragma once

#include "platform.h"
#include "iosfwd.h"

namespace estd { namespace internal {

template <class TChar>
struct setfill
{
    const TChar c;

    ESTD_CPP_CONSTEXPR_RET setfill(TChar c) : c(c) {}
};


struct setw
{
    const unsigned width;

    ESTD_CPP_CONSTEXPR_RET setw(unsigned width) : width(width) {}
};

}

template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& endl(
    detail::basic_ostream<TStreambuf, TBase>& os)
{
    // uses specialized call to bypass sentry so that we don't needlessly check
    // unitbuf and potentially double-flush
    os.put(os.widen('\n'), true);
    os.flush();
    return os;
}

// We have this out here rather than ios_base because of our deviation from virtual methods
template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& dec(detail::basic_ostream<TStreambuf, TBase>& __os)
{
    __os.setf(ios_base::dec, ios_base::basefield);
    return __os;
}

template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& hex(detail::basic_ostream<TStreambuf, TBase>& __os)
{
    __os.setf(ios_base::hex, ios_base::basefield);
    return __os;
}

// TODO: All of the below are noop, feature not fully built out
// NOTE: 'internal' alignment not easy to do since it collides with our namespace

template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& showbase(
    detail::basic_ostream<TStreambuf, TBase>& os)
{
    return os;
}

template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& noshowbase(
    detail::basic_ostream<TStreambuf, TBase>& os)
{
    return os;
}


template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& left(
    detail::basic_ostream<TStreambuf, TBase>& os)
{
    return os;
}

template <class TStreambuf, class TBase>
inline detail::basic_ostream<TStreambuf, TBase>& right(
    detail::basic_ostream<TStreambuf, TBase>& os)
{
    return os;
}



}