#pragma once

#include "internal/platform.h"
#include "typeinfo.h"

namespace estd {

namespace layer0 {

struct type_index
{
    const size_t index;

    template <class T>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    type_index(type_info<T> v) : index(v.hash_code())
    {
    }

    const char* name()
    {
        return experimental::type_name_helper3(index);
    }
};

}

}