#pragma once

#include "../../internal/platform.h"

#ifdef FEATURE_POSIX_IOS

#include <ostream>

#include "../../string.h"

namespace estd
{
// DEBT: Enable this for all compiler scenarios.  For some reason we did have
// to do some filtering.  I don't remember why

// Somehow clang has slightly different expectations during catch << resolution
#if defined(__clang__) || defined(__MINGW32__)
template <class TChar, class TStringTraits, class TAllocator>
std::ostream& operator <<( std::ostream& os,
                           const estd::basic_string<TChar, typename TStringTraits::char_traits, TAllocator, TStringTraits>& value)
{
    // DEBT: This only works for null terminated
    const char* s = value.clock();
    operator <<(os, s);
    value.cunlock();
    return os;
}
#elif defined(FEATURE_ESTD_IOSTREAM_NATIVE) && !defined(__ADSPBLACKFIN__)

//A bit finicky so that we can remove const (via Traits::char_type)
template <class Allocator, class StringTraits,
    class Traits,
    class CharT>
inline std::basic_ostream<typename Traits::char_type, Traits>&
operator<<(std::basic_ostream<typename Traits::char_type, Traits>& os,
    const estd::basic_string<CharT, Traits, Allocator, StringTraits>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
}

/*
template <class CharT, class Traits, class Allocator>
inline std::ostream&
    operator<<(std::ostream& os,
               const estd::basic_string<const CharT, typename Traits::char_traits, Allocator, Traits>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
} */
#endif


}

#endif
