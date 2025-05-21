#pragma once

#include "fwd.h"
#include "traits.h"
#include "../../platform.h"

#include "../../macro/push.h"

namespace estd {

namespace internal {

template<typename T, typename = void>
struct has_destructor : false_type {};

template<typename T>
struct has_destructor<T,
    // Nifty idea, but doesn't seem to work as expected (I presume that void_t is too permissive
    // here)
    //void_t<decltype(std::declval<T&>().~T())>>
    enable_if_t<is_class<T>::value || is_union<T>::value>>
    : true_type {};

template <class Container, class Traits>
class unordered_base : public Traits
{
    using this_type = unordered_base;
    using base_type = Traits;

public:
    using traits = Traits;
    using base_type::key_eq;
    using typename base_type::mapped_type;
    using typename base_type::control_type;
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;
    ESTD_CPP_STD_VALUE_TYPE(typename traits::value_type);

protected:
    ESTD_CPP_CONSTEXPR(14) unordered_base()
    {
        // DEBT: Feels clunky
        for(control_type& v : container_)   base_type::set_null(&v);
    }

    Container container_;

    template <class T, class Enable = enable_if_t<has_destructor<T>::value>>
    static ESTD_CPP_CONSTEXPR(14) void destruct_ll(T& t)
    {
        t.~T();
    }

    template <class T, class Enable = enable_if_t<!has_destructor<T>::value>>
    static ESTD_CPP_CONSTEVAL void destruct_ll(T& t, int = {})
    {
    }

    // runs destructor + nulls out key - does NOT run key destructor
    static void destruct(control_pointer v)
    {
        destruct_ll(traits::mapped(*v));
        base_type::set_null(v);
    }

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
    static ESTD_CPP_CONSTEXPR(14) It skip_empty(It it, It2 end)
    {
        for(; traits::is_empty(*it) && it != end; ++it)   {}

        return it;
    }

    template <class It>
    ESTD_CPP_CONSTEXPR(14) It skip_empty(It it) const
    {
        return skip_empty(it, cast(container_.cend()));
    }

    template <class Control>
    ESTD_CPP_CONSTEXPR(14) Control skip_sparse(Control p, unsigned n) const
    {
        for(; traits::is_sparse(*p, n) && p != container_.cend(); ++p)    {}

        return p;
    }


public:
    using size_type = unsigned;

    struct end_local_iterator
    {
        const_control_pointer it_;
    };

    // The more collisions and/or duplicates you expect, the bigger this wants to be.
    // Idea being if you have two buckets near each other of only size 1, you'll not
    // have room to insert a collision/duplicate in that bucket - and end up in a linear
    // probe overflow (which isn't the end of the world, really)
    // DEBT: Make depth adjustable via traits/impl
    static constexpr unsigned bucket_depth = ESTD_UNORDERED_MAP_BUCKET_SIZE;

    ESTD_CPP_ATTR_NODISCARD
    constexpr size_type max_size() const { return container_.max_size(); }

    ESTD_CPP_ATTR_NODISCARD
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

    // In fact, works for find too but it seems to make things more complicated,
    // not more tidy
    // DEBT: Given above and also the c++17 requirement, not decided if we're gonna keep him
    template <class K, class F, class R = monostate>
    R&& bucket_foreach(const K& key, F&& f, R&& r = monostate{}) const
    {
        const size_type n = bucket(key);
        for(const_local_iterator it = begin(n); it != end(n); ++it)
        {
            if(key_eq_c(key, *it))
            {
                // monostate means no real return type
                if constexpr(!is_same<R, monostate>::value)
                    return std::forward<R>(f(it));
                else
                    f(it);
            }
        }

        return std::forward<R>(r);
    }

    ///
    /// @brief key_eq_c do key_eq on lhs key and rhs control_type
    /// @param k
    /// @param c
    /// @return
    ///
    template <class K, class Control>
    static constexpr bool key_eq_c(const K& k, const Control& c)
    {
        return key_eq()(k, traits::key(c));
    }

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

        // Move over occupied spots.  Sparse does NOT count as occupied
        // DEBT: optimize is_null/is_sparse together
        for(;traits::is_empty(*it) == false; ++it)
        {
            // if we get to the complete end, that's a fail
            // if we've moved to the next bucket, that's also a fail
            if(it == container_.cend() || index(base_type::key(*it)) != n)
                return { nullptr, false };
            else if(!permit_duplicates)
            {
                if(key_eq_c(key, *it))
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

            it_ = parent_->skip_empty(it_);

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
    // DEBT: Inherit from iterator_base, if we can
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
        const_control_pointer control() const { return it_; }

        operator LocalIt() { return cast(); }

        constexpr bool operator==(end_local_iterator it) const
        {
            // If we reach end of entire set, indicate we are at the end
            if(it_ == it.it_)   return true;

            // If we encounter a sparse within this bucket, we expect to skip over him
            // so this is not the end
            if(traits::is_sparse(*it_, n_)) return false;

            // If we reach a null slot, then that's the end of the bucket
            if(parent_->is_empty(*it_)) return true;

            // Reaching here means we have an active slot

            // if n_ doesn't match current key hash, we have reached the end
            // of this bucket
            return n_ != parent_->index(traits::key(*it_));
        }

        constexpr bool operator!=(end_local_iterator it) const
        {
            // If we reach end of entire set, indicate we are NOT NOT at the end
            if(it_ == it.it_)   return false;

            // If we encounter a sparse within this bucket, we expect to skip over him
            // so this is NOT the end
            if(traits::is_sparse(*it_, n_)) return true;

            // If we reach a null slot, that's the end of the bucket - so we fail
            // to assert it's not the end (return false)
            if(parent_->is_empty(*it_)) return false;

            // Reaching here means we have an active slot

            // if n_ matches current key hash, we haven't yet reached the
            // end of this bucket
            return n_ == parent_->index(traits::key(*it_));
        }

        this_type& operator++()
        {
            ++it_;

            // skip over any sparse entries belonging to this bucket.  They are invisible
            // null entries for this iterator
            for(; traits::is_sparse(*it_, n_) && it_ != parent_->container_.cend(); ++it_)   {}

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
    ESTD_CPP_CONSTEXPR(14) find_result<const_control_pointer> find_ll(const K& x) const
    {
        const size_type n = index(x);

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq_c(x, *it))    return { it.control(), n };

        return { container_.cend(), npos() };
    }

    template <class K>
    ESTD_CPP_CONSTEXPR(14) find_result<control_pointer> find_ll(const K& x)
    {
        const size_type n = index(x);

        for(local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq_c(x, *it))    return { it.control(), n };

        return { container_.end(), npos() };
    }

    // deviates from std in that other iterators part of this bucket could be invalidated
    void erase_and_gc_ll(control_pointer pos)
    {
        auto start = pos;

        destruct(pos);

        ++pos;

        // No housekeeping if next guy is null (sparse is OK, but
        // not yet accounted for here)
        if (base_type::is_empty(*pos))
            return;

        // Quick-deduce our bucket#
        size_type n = start - container_.cbegin();
        // Find last one in bucket
        for(;n == index(traits::key(*pos)) && pos < container_.cend(); ++pos) {}
        // Decrement to position on actual last one in bucket
        --pos;

        swap(*pos, *start);
    }

public:
    template <class K>
    size_type count(const K& x) const
    {
        unsigned counter = 0;
        // DEBT: Depends on c++17
        bucket_foreach(x, [&](const_local_iterator) { ++counter; });
        return counter;
    }

    template <class K>
    constexpr bool contains(const K& key) const
    {
        return find_ll(key).second != npos();
    }

    ESTD_CPP_CONSTEXPR(14) bool empty() const   // NOLINT
    {
        for(const control_type& v : container_)
            if(traits::is_empty(v) == false) return false;

        return true;
    }

    ESTD_CPP_CONSTEXPR(14) size_type size() const   // NOLINT
    {
        // Not doing estd::distance approach, a bit more efficient to skip 'iterator' usage
        size_type sz = 0;

        for(const control_type& v : container_)
            if(traits::is_empty(v) == false)  ++sz;

        return sz;
    }

    // DEBT: May be a deviation since our buckets are a little more fluid, but I think
    // it conforms to spec
    template <class K>
    constexpr size_type bucket(const K& key) const
    {
        return index(key);
    }

    using iterator = iterator_base<pointer, this_type>;
    using const_iterator = iterator_base<const_pointer, this_type>;
    using local_iterator = local_iterator_base<pointer>;
    using const_local_iterator = local_iterator_base<const_pointer>;

    iterator begin()
    {
        return { this, skip_empty(cast(container_.begin())) };
    }

    constexpr const_iterator begin() const
    {
        return { this, skip_empty(cast(container_.cbegin())) };
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

    // FIX: These guys need to skip over sparse
    local_iterator begin(size_type n)
    {
        return { this, n, skip_sparse(&container_[n], n) };
    }

    constexpr const_local_iterator begin(size_type n) const
    {
        return { this, n, skip_sparse(&container_[n], n) };
    }

    constexpr const_local_iterator cbegin(size_type n) const
    {
        return { this, n, skip_sparse(&container_[n], n) };
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

#include "../../macro/pop.h"
