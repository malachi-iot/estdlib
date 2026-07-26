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

template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& endl(
    detail::basic_ostream<Streambuf, Base>& os)
{
    // uses specialized call to bypass sentry so that we don't needlessly check
    // unitbuf and potentially double-flush
    os.put(os.widen('\n'), true);
    os.flush();
    return os;
}

template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& uppercase(
    detail::basic_ostream<Streambuf, Base>& os)
{
    os.setf(os.uppercase);
    return os;
}

template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& nouppercase(
    detail::basic_ostream<Streambuf, Base>& os)
{
    os.unsetf(os.uppercase);
    return os;
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

// 26JUL26 MB DEBT: MAY work, but untested
template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& left(
    detail::basic_ostream<Streambuf, Base>& os)
{
    os.setf(os.left);
    return os;
}

// 26JUL26 MB DEBT: MAY work, but untested
template <class Streambuf, class Base>
inline detail::basic_ostream<Streambuf, Base>& right(
    detail::basic_ostream<Streambuf, Base>& os)
{
    os.setf(os.right);
    return os;
}



}
