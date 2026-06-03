#pragma once

#include "fwd.h"

#include "../iterator_standalone.h"
#include "../raw/memory.h"
#include "../raw/type_traits.h"

namespace estd {

#if FEATURE_ESTD_STD_MEMORY_ALIAS
using std::uninitialized_copy;
using std::uninitialized_copy_n;
#else
// Adapted from https://en.cppreference.com/cpp/memory/uninitialized_copy
template<class InputIt, class ForwardIt>
ESTD_CPP_CONSTEXPR(14) ForwardIt uninitialized_copy(InputIt first, InputIt last,
    ForwardIt d_first)
{
    using value_type = typename iterator_traits<ForwardIt>::value_type;
    for (; first != last; ++first, ++d_first)
        return ::new (addressof(*d_first)) value_type(*first);

    return d_first;
}
#endif

}