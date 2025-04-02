#pragma once

#include "../../platform.h"

namespace estd {

namespace internal {

// DEBT: This guy wants to play with estd::layer1::optional you can feel it
// We are doing this rather than a Null = T{} from the get go because some T won't
// play nice in that context
// TODO: layer1::string ought to only operate on value[0] null termination here, which is
// an ideal usage.  However it MAY be overzealously setting deeper chars to null.  Doublecheck
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


template <class Key, class Hash, class KeyEqual, class Nullable = nullable_traits<Key>>
class unordered_traits :
    // NOTE: Although EBO is interesting, spec suggests they aren't relevant here
    // i.e. https://en.cppreference.com/w/cpp/container/unordered_map/hash_function
    protected Hash,  // EBO
    protected KeyEqual  // EBO
{
public:
    using key_type = Key;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using nullable = nullable_traits<Key>;

    //hasher& hash_function() { return *this; }
    //constexpr const hasher& hash_function() const { return *this; }
    //hasher hash_function() const { return {}; }
};


}

}
