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
    const char* s = value.clock();
    operator <<(os, s);
    value.cunlock();
    return os;
}
#endif
}

#endif
