#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"

namespace estd {

namespace internal {

// DEBT: Hard-wired to layer1 stlye

template <
    unsigned sz,
    class Key,
    class T,
    class Hash = hash<Key>,
    Key Null = Key(),
    class KeyEqual = equal_to<Key> >
class unordered_map;

template <unsigned N, class Key, class T, class Hash, Key Null, class KeyEqual>
class unordered_map :
    protected Hash,  // EBO
    protected KeyEqual  // EBO
{
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = estd::pair<const Key, T>;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using hasher = Hash;
    using size_type = unsigned;

private:
    value_type container_[N];

public:
    // NOTE: Not sure if end/cend represents end of raw container or end of active, useful
    // buckets but I think it's the former
    constexpr const_iterator cend() const { return &container_[N]; }

    static constexpr size_type max_size() { return N; }

    pair<iterator, bool> insert(const_reference value)
    {
        unsigned hashed = hasher{}(value.first) % max_size();

        // linear probing
        iterator it = &container_[hashed];
        while(it->first != Null)
        {
            if(++it == cend())
            {
                return { it, false };
            }
        }

        //*it = value;

        return { it, true };
    }
};

}

}
