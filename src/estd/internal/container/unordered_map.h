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

    using end_local_iterator = monostate;

    //using local_iterator = iterator;
    //using const_local_iterator = const_iterator;

private:
    // FIX: We need this semi-initialized, with keys all being Null
    uninitialized_array<value_type, N> container_;

    // DEBT: Doesn't handle non-empty hasher
    static constexpr size_type index(const key_type& key)
    {
        return hasher{}(key) % max_size();
    }

    // indicates whether already-hashed (lhs) matches to-hash (rhs)
    static constexpr bool match(size_type lhs, const key_type& rhs)
    {
        return lhs == index(rhs);
    }

    template <class LocalIt>
    struct local_iterator_base
    {
        using this_type = local_iterator_base;

        // bucket designator
        const size_type n_;

        LocalIt it_;

        constexpr value_type operator*() const { return *it_; }

        constexpr const_pointer operator->() const { return it_; }

        constexpr bool operator!=(end_local_iterator) const
        {
            // if n_ matches current key hash, we haven't yet reached the
            // end of this bucket
            return n_ == index(it_->first);
        }

        this_type& operator++()
        {
            ++it_;
            return *this;
        }
    };

public:
    using local_iterator = local_iterator_base<iterator>;
    using const_local_iterator = local_iterator_base<const_iterator>;

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

    local_iterator begin(size_type n)
    {
        return { n, &container_[n] };
    }

    const_local_iterator cbegin(size_type n) const
    {
        return { n, &container_[n] };
    }

    static constexpr end_local_iterator end(size_type)
    {
        return {};
    }

    // NOTE: This works, but you'd prefer to avoid it and iterate yourself directly
    size_type bucket_size(size_type n) const
    {
        unsigned counter = 0;

        for(const_local_iterator it = cbegin(n); it != end(n); ++it)
            ++counter;

        return counter;
    }

    constexpr bool contains(const key_type key) const
    {
        return container_[index(key)].first != Null;
    }
};

}

}
