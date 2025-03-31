#pragma once

#include "traits.h"

namespace estd {

namespace internal {

template <class Key, class Hash, class KeyEqual>
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

    //hasher& hash_function() { return *this; }
    //constexpr const hasher& hash_function() const { return *this; }
    //hasher hash_function() const { return {}; }
};

template <class Traits>
class unordered_base : public Traits
{
protected:
    //Container container_;

public:
    using size_type = unsigned;

    // The more collisions and/or duplicates you expect, the bigger this wants to be.
    // Idea being if you have two buckets near each other of only size 1, you'll never
    // have room to insert a collision/duplicate
    static constexpr unsigned bucket_depth = 4;
};

template <unsigned N, class Traits>
class l1_unordered_base : public unordered_base<Traits>
{
    using base_type = unordered_base<Traits>;

    // DEBT: Do assert to make sure N is evenly divisible by bucket_depth

public:
    using typename base_type::size_type;
    using base_type::bucket_depth;

    static constexpr size_type max_bucket_count()
    {
        return N / bucket_depth;
    }

    static constexpr size_type max_size() { return N; }

protected:
    // DEBT: Doesn't handle non-empty hasher
    template <class K>
    static constexpr size_type index(const K& key)
    {
        return bucket_depth * typename base_type::hasher{}(key) % max_size();
    }

    // indicates whether already-hashed (lhs) matches to-hash (rhs)
    static constexpr bool match(size_type lhs, const typename base_type::key_type& rhs)
    {
        return lhs == index(rhs);
    }

    // rehash: increasing size of a bucket
    void rehash_up()
    {

    }

    // rehash: decreasing size of a bucket
    void rehash_down()
    {

    }
};


}

}
