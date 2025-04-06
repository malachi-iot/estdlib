#pragma once

#include "fwd.h"
#include "traits.h"
#include "../../platform.h"

namespace estd {

namespace internal {

template <class Container, class Traits>
class unordered_base : public Traits
{
    using this_type = unordered_base;
    using base_type = Traits;
    using traits = Traits;

public:
    using base_type::key_eq;
    using typename base_type::control_type;
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;
    ESTD_CPP_STD_VALUE_TYPE(typename traits::value_type);

protected:
    Container container_;

    static constexpr pointer cast(control_pointer p)
    {
        return reinterpret_cast<pointer>(p);
    }

    static constexpr const_pointer cast(const_control_pointer p)
    {
        return reinterpret_cast<const_pointer>(p);
    }

    // 'It' must be control_pointer or const_control_pointer
    // DEBT: Get rid of 'It2'
    template <class It, class It2>
    static ESTD_CPP_CONSTEXPR(14) It skip_null(It it, It2 end)
    {
        for(; traits::is_null_or_sparse(*it) && it != end; ++it)   {}

        return it;
    }

    template <class It>
    ESTD_CPP_CONSTEXPR(14) It skip_null(It it) const
    {
        return skip_null(it, cast(container_.cend()));
    }


public:
    using size_type = unsigned;

    struct end_local_iterator
    {
        const_control_pointer it_;
    };

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

    using insert_result = pair<control_pointer, bool>;

    ///
    /// @tparam K
    /// @param key
    /// @param permit_duplicates
    /// @return control_pointer + true = success.
    ///         nullptr + false = no insert can occur.
    ///         control_pointer + false = duplicate found, and here it is
    template <class K>
    insert_result insert_precheck(const K& key, bool permit_duplicates)
    {
        const size_type n = index(key);

        // linear probing

        control_pointer it = &container_[n];

        // Move over occupied spots.  Sparse also counts as occupied
        // DEBT: optimize is_null/is_sparse together
        for(;traits::is_null_or_sparse(*it) == false || traits::is_sparse(*it, n); ++it)
        {
            // if we get to the complete end, that's a fail
            // if we've moved to the next bucket, that's also a fail
            if(it == container_.cend() || index(base_type::key(*it)) != n)
                return { nullptr, false };
            else if(!permit_duplicates)
            {
                if(traits::key_eq(key, *it))
                    // "value set to true if and only if the insertion took place."
                    return { it, false };
            }

            // Unlike std::unordered_map, we don't always kick back duplicate keys.
            // Instead, that's undefined behavior if you try to pull via [],
            // but iterating through a bucket you can get to all of them (and more, likely)
        }

        // Success, but someone else still needs to initialize 'it'
        return { it, true };
    }


    // semi-smart, can skip null spots
    template <class It, class Parent>
    class iterator_base
    {
        using parent_type = Parent;
        using this_type = iterator_base;

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

    // Skips null spots, omits sparse guys and ends if we go outside of bucket
    template <class LocalIt>
    struct local_iterator_base
    {
        using parent_type = unordered_base;
        using this_type = local_iterator_base;

        const parent_type* const parent_;

        // bucket designator
        const size_type n_;

        const_control_pointer it_;

        constexpr LocalIt cast() const
        {
            // DEBT: Do static assert to verify convertibility

            return (LocalIt) it_;
        }

        constexpr const_reference operator*() const { return *cast(); }

        constexpr const_pointer operator->() const { return cast(); }

        // DEBT: Effectively a const_cast
        control_pointer control() { return (control_pointer)it_; }

        operator LocalIt() { return cast(); }

        constexpr bool operator==(end_local_iterator it) const
        {
            // If we reach end of entire set, indicate we are at the end
            if(it_ == it.it_)   return true;

            // If we reach a null slot, then that's the end of the bucket
            if(parent_->is_null_or_sparse(*it_)) return true;

            // if n_ doesn't match current key hash, we have reached the end
            // of this bucket
            return n_ != parent_->index(traits::key(*it_));
        }

        constexpr bool operator!=(end_local_iterator it) const
        {
            // If we reach end of entire set, indicate we are NOT NOT at the end
            if(it_ == it.it_)   return false;

            // If we reach a null slot, that's the end of the bucket - so we fail
            // to assert it's not the end (return false)
            if(parent_->is_null_or_sparse(*it_)) return false;

            // if n_ matches current key hash, we haven't yet reached the
            // end of this bucket
            return n_ == parent_->index(traits::key(*it_));
        }

        this_type& operator++()
        {
            ++it_;

            // skip over any sparse entries belonging to this bucket.  They are invisible
            // null entries for this iterator
            for(; parent_->is_sparse(*it_, n_) && it_ != parent_->container_.cend(); ++it_)   {}

            return *this;
        }

        this_type operator++(int)
        {
            operator++();

            return { n_, it_ - 1 };
        }

        constexpr bool operator==(const LocalIt& other) const
        {
            return it_ == other;
        }

        constexpr bool operator!=(const LocalIt& other) const
        {
            return it_ != other;
        }
    };

    static constexpr size_type npos() { return numeric_limits<size_type>::max(); }

    // pointer and bucket
    template <class Pointer>
    using find_result = pair<Pointer, size_type>;

    template <class K>
    ESTD_CPP_CONSTEXPR(14) find_result<const_pointer> find_ll(const K& x) const
    {
        const size_type n = index(x);

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq(x, *it))    return { it.cast(), n };

        return { cast(container_.cend()), npos() };
    }

    template <class K>
    ESTD_CPP_CONSTEXPR(14) find_result<pointer> find_ll(const K& x)
    {
        const size_type n = index(x);

        for(local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq(x, *it))    return { it.cast(), n };

        return { cast(container_.end()), npos() };
    }


public:
    template <class K>
    constexpr bool contains(const K& key) const
    {
        return find_ll(key).second != npos();
    }

    ESTD_CPP_CONSTEXPR(14) bool empty() const
    {
        for(const control_type& v : container_)
            if(traits::is_null_or_sparse(v) == false) return false;

        return true;
    }

    ESTD_CPP_CONSTEXPR(14) size_type size() const
    {
        // Not doing estd::distance approach, a bit more efficient to skip 'iterator' usage
        size_type sz = 0;

        for(const control_type& v : container_)
            if(traits::is_null_or_sparse(v) == false)  ++sz;

        return sz;
    }


    // Not directly used yet
    using iterator = iterator_base<pointer, this_type>;
    using const_iterator = iterator_base<const_pointer, this_type>;
    using local_iterator = local_iterator_base<pointer>;
    using const_local_iterator = local_iterator_base<const_pointer>;

    iterator begin()
    {
        return { this, skip_null(cast(container_.begin())) };
    }

    constexpr const_iterator begin() const
    {
        return { this, skip_null(cast(container_.cbegin())) };
    }

    // DEBT: can probably use a hard type like end_iterator (optimization) though
    // that does double down on carrying parent* around
    constexpr const_iterator end() const
    {
        return { this, cast(container_.cend())  };
    }

    constexpr const_iterator cend() const
    {
        return { this, cast(container_.cend()) };
    }

    local_iterator begin(size_type n)
    {
        return { this, n, &container_[n] };
    }

    constexpr const_local_iterator begin(size_type n) const
    {
        return { this, n, &container_[n] };
    }

    constexpr const_local_iterator cbegin(size_type n) const
    {
        return { this, n, &container_[n] };
    }

    constexpr end_local_iterator end(size_type) const
    {
        return { container_.cend() };
    }

    constexpr end_local_iterator cend(size_type) const
    {
        return { container_.cend() };
    }
};

}

}
