#pragma once

// DEBT: Eventually Container is passed into these base classes and this guy
// won't be needed
#include "../../array.h"

#include "fwd.h"
#include "traits.h"

namespace estd {

namespace internal {

template <class Traits>
class unordered_base : public Traits
{
    using base_type = Traits;
    using traits = Traits;

protected:
    //Container container_;

public:
    using typename traits::mapped_type;
    using typename traits::nullable;
    using size_type = unsigned;

    // The more collisions and/or duplicates you expect, the bigger this wants to be.
    // Idea being if you have two buckets near each other of only size 1, you'll never
    // have room to insert a collision/duplicate
    static constexpr unsigned bucket_depth = 4;

    // Mainly used for unordered_map since it has an unused area when key is null
    union meta
    {
        byte storage[sizeof(mapped_type)];

        struct
        {
            // aka "sparse" - exists specifically to mark as deleted, but physically unmoved
            uint16_t marked_for_gc : 1;
            // which bucket this empty slot *used to* belong to
            uint16_t bucket : 6;
        };

        operator mapped_type& () { return * (mapped_type*) storage; }
        constexpr operator const mapped_type& () const { return * (mapped_type*) storage; }

        mapped_type& mapped() { return * (mapped_type*) storage; }
    };

    using map_control_type = estd::pair<typename traits::key_type, meta>;

    /// @brief Checks for null OR sparse
    /// @param v
    /// @return
    template <class K, class T2>
    static constexpr bool is_null_or_spase(const pair<K, T2>& v)
    {
        return nullable{}.is_null(v.first);
    }
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
    // DEBT: Temporarily located here
    uninitialized_array<typename base_type::map_control_type, N> container_;

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
