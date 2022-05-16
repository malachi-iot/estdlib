#pragma once

#include "../algorithm.h"

namespace estd { namespace internal {

// AVR compiler appears to have incomplete implementation of __has_builtin
// DEBT: According to https://stackoverflow.com/questions/3944505/detecting-signed-overflow-in-c-c
// contents of the result (&__val in this case) are undefined on overflow.  That might be an
// issue down the line if we're depending on what we've gathered so far.
#if defined(__has_builtin) && !defined(__AVR__)
// adapted from GNUC
template<typename _Tp>
typename estd::enable_if<estd::is_signed<_Tp>::value, bool>::type
inline raise_and_add(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_mul_overflow(__val, __base, &__val)
        || __builtin_add_overflow(__val, __c, &__val))
        return false;
    return true;
}


template<typename _Tp>
typename estd::enable_if<!estd::is_signed<_Tp>::value, bool>::type
inline raise_and_add(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_umul_overflow(__val, __base, &__val)
        || __builtin_uadd_overflow(__val, __c, &__val))
        return false;
    return true;
}

template<typename _Tp>
typename estd::enable_if<estd::is_signed<_Tp>::value, bool>::type
inline raise_and_sub(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_mul_overflow(__val, __base, &__val)
        || __builtin_sub_overflow(__val, __c, &__val))
        return false;
    return true;
}
#else
// Overflow goes unnoticed.  If the __builtins aren't available, you may have to implement
// a platform specific approach here
template <typename T>
inline bool raise_and_add(T& val, const unsigned short base, unsigned char c)
{
    val *= base;
    val += c;
    return true;
}


template <typename T>
inline bool raise_and_sub(T& val, const unsigned short base, unsigned char c)
{
    val *= base;
    val -= c;
    return true;
}
#endif

}}