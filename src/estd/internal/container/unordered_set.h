#pragma once

#include "../platform.h"
#include "../fwd/functional.h"

namespace estd { namespace internal {

template <class Container,
    class Key = typename Container::value_type,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key> >
class unordered_set;

template <class Container, class Key, class Hash, class KeyEqual>
class unordered_set :
    protected Hash,  // EBO
    protected KeyEqual  // EBO
{
    Container set_;

public:
    ESTD_CPP_STD_VALUE_TYPE(Key)

    using key_type = value_type;
    using hasher = Hash;
    using size_type = unsigned;

    constexpr size_type size() const { return set_.size(); }
    constexpr size_type max_size() const { return set_.max_size(); }
};

}}
