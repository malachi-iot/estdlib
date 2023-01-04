#pragma once

#include "../internal/fwd/ostream.h"

namespace estd { namespace internal {

#if __cplusplus < 201103L
// DEBT: Move all this out to c++03 specific area
template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, int value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, unsigned value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, long value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, unsigned long value)
{
    return out_int_helper(out, value);
}

#endif

}}