#pragma once

#include "../../platform.h"

namespace estd {

namespace internal {

// DEBT: This guy wants to play with estd::layer1::optional you can feel it
// We are doing this rather than a Null = T{} from the get go because some T won't
// play nice in that context
template <class T>
struct nullable_traits
{
    static constexpr bool is_null(const T& value)
    {
        return value == T{};
    }

    // DEBT: Don't want to use this, just helpful for init and erase
    static constexpr T get_null() { return T(); }

    ESTD_CPP_CONSTEXPR(17) static void set(T* value)
    {
        *value = T{};
    }
};


}

}
