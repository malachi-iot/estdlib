#pragma once

#include "fwd.h"
#include "traits.h"

namespace estd {

namespace internal {

template <class Key, class Hash, class KeyEqual, class Nullable>
struct unordered_set_traits : unordered_traits<Key, Key, Hash, KeyEqual, Nullable>
{

};


template <class Container, class Traits>
class unordered_base : public Traits
{
    using base_type = Traits;
    using traits = Traits;

protected:
    Container container_;

public:
    using size_type = unsigned;

    // The more collisions and/or duplicates you expect, the bigger this wants to be.
    // Idea being if you have two buckets near each other of only size 1, you'll never
    // have room to insert a collision/duplicate
    static constexpr unsigned bucket_depth = 4;

    constexpr size_type max_size() const { return container_.max_size(); }

    constexpr size_type max_bucket_count() const
    {
        return max_size() / bucket_depth;
    }

protected:
    // DEBT: Doesn't handle non-empty hasher
    template <class K>
    constexpr size_type index(const K& key) const
    {
        return bucket_depth * typename base_type::hasher{}(key) % max_size();
    }

    // indicates whether already-hashed (lhs) matches to-hash (rhs)
    constexpr bool match(size_type lhs, const typename base_type::key_type& rhs) const
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

    // semi-smart, can skip null spots
    template <class It, class Parent>
    class iterator_base
    {
        using parent_type = Parent;
        using this_type = iterator_base;
        using pointer = typename parent_type::pointer;
        using const_pointer = typename parent_type::const_pointer;
        using reference = typename parent_type::reference;
        using const_reference = typename parent_type::const_reference;

        const parent_type* parent_;
        It it_;

        template <class OtherIt, class Parent2>
        friend class iterator_base;

    public:
        constexpr iterator_base(const parent_type* parent, It it) :
            parent_{parent},
            it_{it}
        {}

        iterator_base(const iterator_base&) = default;

        this_type& operator++()
        {
            ++it_;

            it_ = parent_->skip_null(it_);

            return *this;
        }

        constexpr const_reference operator*() const { return *it_; }

        constexpr const_pointer operator->() const { return it_; }

        pointer operator->() { return it_; }

        // DEBT: temporary as we transition container_
        pointer value() { return it_; }
        constexpr const_pointer value() const { return it_; }

        template <class OtherIt>
        constexpr bool operator==(const iterator_base<OtherIt, Parent>& other) const
        {
            return it_ == other.it_;
        }

        template <class OtherIt>
        constexpr bool operator!=(const iterator_base<OtherIt, Parent>& other) const
        {
            return it_ != other.it_;
        }

        constexpr bool operator==(const It& other) const
        {
            return it_ == other;
        }

        constexpr bool operator!=(const It& other) const
        {
            return it_ != other;
        }
    };
};

}

}
