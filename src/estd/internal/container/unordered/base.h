#pragma once

#include "traits.h"

namespace estd {

namespace internal {

template <class Key, class Hash, class KeyEqual>
class unordered_base :
    protected Hash,  // EBO
    protected KeyEqual  // EBO
{
protected:
    //Container container_;

public:
    using key_type = Key;
    using hasher = Hash;
    using key_equal = KeyEqual;
};

}

}
