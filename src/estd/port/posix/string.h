#pragma once

#include "../../internal/platform.h"

// 11APR25 DEBT: Remove this guy and rethink our "POSIX" naming (I was interchanging
// that with stdlib before)

#ifdef FEATURE_POSIX_IOS

#include <ostream>

#include "../../string.h"

namespace estd
{
// DEBT: Enable this for all compiler scenarios.  For some reason we did have
// to do some filtering.  I don't remember why

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

}

#endif
