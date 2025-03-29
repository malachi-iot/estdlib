#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"

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
    
    using local_iterator = iterator;
    using const_local_iterator = const_iterator;

private:
    uninitialized_array<value_type, N> container_;

    constexpr size_type index(const key_type& key) const
    {
        return hasher{}(key) % max_size();
    }

public:
    // NOTE: Not sure if end/cend represents end of raw container or end of active, useful
    // buckets but I think it's the former
    constexpr const_iterator cend() const { return container_.end(); }

    static constexpr size_type max_size() { return N; }

    // DEBT: May be a deviation since our buckets are a little more fluid, but I think
    // it conforms to spec
    constexpr size_type bucket(const key_type& key) const
    {
        return index(key);
    }

    mapped_type& operator[](const key_type& key)
    {
        return container_[index(key)].second;
    }

    pair<iterator, bool> insert(const_reference value)
    {
        unsigned hashed = index(value.first);

        // linear probing
        iterator it = &container_[hashed];
        while(it->first != Null)
        {
            if(++it == cend())
            {
                return { it, false };
            }
        }

        new (it) value_type(value);

        return { it, true };
    }

    /*
    size_type bucket_size(size_type n) const
    {

    }   */

    local_iterator begin(size_type n)
    {
        return &container_[n];
    }

    const_local_iterator end(size_type n) const
    {
        // TBD
        return {};
    }
};

}

}
