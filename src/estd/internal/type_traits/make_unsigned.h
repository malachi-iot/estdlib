#pragma once

// DEBT: Really we should include full limits, but as we develop make_unsigned
// I want to keep dependencies small as possible
#include "../fwd/limits.h"

namespace estd {

template <class T>
struct make_unsigned
{
    typedef typename numeric_limits<T>::unsigned_type type;
};

#if __cpp_alias_templates
template <class T>
using make_unsigned_t = typename make_unsigned<T>::type;
#endif

}
