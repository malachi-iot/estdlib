#pragma once

#include "../string.h"

namespace estd {
namespace internal {

template <class TStreambuf, class TBase>
struct basic_ostream;

}
template <class TStreambuf, class TBase, class TStringAllocator, class TStringPolicy>
inline internal::basic_ostream<TStreambuf, TBase>& operator <<(
        internal::basic_ostream<TStreambuf, TBase>& out,
        const estd::basic_string<
            typename TStreambuf::char_type,
            typename TStreambuf::traits_type,
            TStringAllocator,
            TStringPolicy >& str
        )
{
    out.write(str.clock(), str.size());

    str.cunlock();

    return out;
}

}
