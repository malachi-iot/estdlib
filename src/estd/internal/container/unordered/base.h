#pragma once

#include "fwd.h"
#include "swap.h"
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

// To help ease c++11's nerves
struct unordered_base_constants
{
    using size_type = unsigned;

    static constexpr size_type npos = numeric_limits<size_type>::max();
};


// DEBT: Split up the logic in this class if we can so that we don't have such a massive line count
template <class Container, class Traits>
class unordered_base : public Traits,
    public unordered_base_constants
{
    using this_type = unordered_base;
    using base_type = Traits;

public:
    using traits = Traits;
    using base_type::key_eq;
    using typename base_type::mapped_type;
    using typename base_type::control_type;
    using typename base_type::nullable;

    // As mentioned elsewhere, 'control_type' and 'value_type' are basically union-ized
    // We discriminate which is which based on 'key' null state

    // control_type is set up in traits.  It is used for non-allocated
    // entries and is generally:
    // 1.  For unordered_map = pair<key, meta>
    // 2.  For unordered_set = key
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;

    // traits::value type (and therefore 'pointer') is used for allocated entries:
    // 1.  For unordered_map = pair<const key, mapped_type>
    // 2.  For unordered_set = key;
    ESTD_CPP_STD_VALUE_TYPE(typename traits::value_type)

protected:
    template <class ...Args>
    ESTD_CPP_CONSTEXPR(14) explicit unordered_base(Args&&...args) :
        container_{std::forward<Args>(args)...}
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
    static ESTD_CPP_CONSTEXPR(14) void destruct_ll(const T&, int = {})
    {
    }

    // runs destructor + nulls out key - does NOT run key destructor
    static ESTD_CPP_CONSTEXPR(14) void destruct(control_pointer v)
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
        // FIX: Needs wraparound treatment
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
        // FIX: Needs wraparound treatment
        for(; traits::is_sparse(*p, n) && p != container_.cend(); ++p)    {}

        return p;
    }


public:
    struct end_local_iterator
    {
        const_control_pointer it_;
    };

    // The more collisions and/or duplicates you expect, the bigger this wants to be.
    // Idea being if you have two buckets near each other of only size 1, you'll not
    // have room to insert a collision/duplicate in that bucket - and end up in a linear
    // probe overflow (which isn't the end of the world, really)
    static constexpr unsigned bucket_depth = traits::bucket_depth;

    ESTD_CPP_ATTR_NODISCARD
    constexpr size_type max_size() const { return container_.size(); }

    ESTD_CPP_ATTR_NODISCARD
    constexpr size_type max_bucket_count() const
    {
        return max_size() / bucket_depth;
    }

protected:
    /// Given a key, return the physical index one may use in the container
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
    // 18JUN26 MB DEBT: I don't even remember what I am doing here.
    // Why am I returning under one path but not the other?
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
    /// @remarks behavior undefined when key is null
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
    ESTD_CPP_CONSTEXPR(14) insert_result insert_precheck(const K& key, bool permit_duplicates)
    {
        constexpr insert_result null { nullptr, false };

#if ESTD_UNORDERED_MAP_STRICT
        if(nullable::is_null(key))  return null;
#endif

        const size_type n = index(key);

        // linear probing-ish
        // See https://github.com/malachi-iot/estdlib/issues/211

        control_pointer it = &container_[n];

#if ISSUE_211_BRINGUP
        control_pointer start = it;
#endif

        // Move over occupied spots.  Sparse does NOT count as occupied
        // DEBT: optimize is_null/is_sparse together
        while(traits::is_empty(*it) == false)
        {
            // if we get to the complete end, that's a fail

#if !ISSUE_211_BRINGUP
            // if we've moved to the next bucket, that's also a fail
            if(index(base_type::key(*it)) != n)
                return null;
#endif

            if(!permit_duplicates)
            {
                if(key_eq_c(key, *it))
                    // "value set to true if and only if the insertion took place."
                    return { it, false };
            }

#if ISSUE_211_BRINGUP
            if(++it == container_.cend())
                it = &container_[0];

            if(it == start) return null;
#else
            if(++it == container_.cend())
                return null;
#endif

            // Unlike std::unordered_map, we don't always kick back duplicate keys.
            // Instead, that's undefined behavior if you try to pull via [],
            // but iterating through a bucket you can get to all of them (and more, likely)
        }

        // Success, but someone else still needs to initialize 'it'
        return { it, true };
    }


    // semi-smart, can skip null spots
    template <class Value, class Parent = this_type>
    class iterator_base
    {
        using parent_type = Parent;
        using this_type = iterator_base;

        // these two may or may not be const
        using value_type = Value;
        using pointer = value_type*;

        const parent_type* parent_;
        pointer it_;

        template <class OtherIt, class Parent2>
        friend class iterator_base;

    public:
        constexpr iterator_base() = default;
        constexpr iterator_base(const parent_type* parent, pointer it) :
            parent_{parent},
            it_{it}
        {}

        iterator_base(const iterator_base&) = default;

        // Assist to enable iterator_base to comfortably demote to const_iterator_base
        // 21JUN26 MB DEBT: Tighten up rules here
        template <class Value2>
        constexpr iterator_base(const iterator_base<Value2, Parent>& copy_from) :
            parent_{copy_from.parent_},
            it_{copy_from.it_}
        {}

        this_type& operator++()
        {
            ++it_;

            it_ = parent_->skip_empty(it_);

            return *this;
        }

        constexpr const_reference operator*() const { return *it_; }
        ESTD_CPP_CONSTEXPR(14) value_type& operator*() { return *it_; }

        constexpr const_pointer operator->() const { return it_; }
        ESTD_CPP_CONSTEXPR(14) pointer operator->() { return it_; }

        template <class Value2>
        constexpr bool operator==(const iterator_base<Value2, Parent>& other) const
        {
            return it_ == other.it_;
        }

        template <class Value2>
        constexpr bool operator!=(const iterator_base<Value2, Parent>& other) const
        {
            return it_ != other.it_;
        }

        constexpr bool operator==(const_pointer& other) const
        {
            return it_ == other;
        }

        constexpr bool operator!=(const_pointer& other) const
        {
            return it_ != other;
        }
    };

    // Iterates through all active spots in a bucket
    // Skips null spots, omits sparse guys and ends if we go outside of bucket
    // 20JUN26 MB: For linear probing upgrade (https://github.com/malachi-iot/estdlib/issues/211)
    //  'outside the bucket' actually means reaching a null spot.  Also, "skips null spots"
    //  is misleading.  We actually terminate at null spots.
    // DEBT: Inherit from iterator_base, if we can
    template <class LocalIt>
    struct local_iterator_base
    {
        using parent_type = unordered_base;
        using this_type = local_iterator_base;

        const parent_type* const parent_;

        // DEBT: Do this to hide non-const control() guy
        //friend parent_type;

        // bucket designator
        const size_type n_;

        const_control_pointer it_;

#if ISSUE_211_BRINGUP
        // DEBT: Crude wraparound detect helper
        const_control_pointer start_{nullptr};

        constexpr bool started() const { return start_ != nullptr; }
#endif

        LocalIt cast() const
        {
            // DEBT: Do static assert to verify convertibility
            //static_assert(is_same<remove_cv_t<LocalIt>, pointer>::value);
            //static_assert(is_same<add_const_t<LocalIt>, const_pointer>::value);

            return (LocalIt) it_;
        }

        const_reference operator*() const { return *cast(); }

        const_pointer operator->() const { return cast(); }

        control_pointer control() { return const_cast<control_pointer>(it_); }
        constexpr const_control_pointer control() const { return it_; }

        operator LocalIt() { return cast(); }

        constexpr bool is_empty() const
        {
            return parent_->is_empty(*it_);
        }

#if ISSUE_211_BRINGUP
        // Enhanced linear probing flavor puts more validity burden
        // on skip_sparse_and_foreign_buckets, resulting in a simplified
        // equality operator
        ESTD_CPP_CONSTEXPR(14) bool operator==(end_local_iterator it) const
        {
            // If we reach a null slot, then that's the end of probing
            if(is_empty()) return true;

            // Otherwise if we wrap around, that's also the end
            return started() && it_ == it.it_;
        }

        ESTD_CPP_CONSTEXPR(14) bool operator!=(end_local_iterator it) const
        {
            // If we reach a null slot, then that's the end of probing
            if(is_empty()) return false;

            // If we haven't bumped forward, there's no chance of wraparound
            // so we can't be at the end
            if(!started()) return true;

            // If we wrap around, that's also the end
            return it_ != it.it_;
        }
#else
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
#endif

#if ISSUE_211_BRINGUP
        void bump_with_rollover()
        {
            // DEBT: Really obnoxious wraparound detect assist
            if(start_ == nullptr)   start_ = it_;

            if(++it_ == parent_->container_.cend()) it_ = &parent_->container_[0];
        }

        // return false = rolled over, total end
        // return true = it_ is valid
        bool skip_sparse_and_foreign_buckets()
        {
            //const_control_pointer start = &parent_->container_[n_];
            // Linear probing dictates we may encounter foreign bucket hashes along the way.  They
            // do not constitute the end of a bucket.  Only a null entry or a rollover is the end
            // of a bucket.
            while(it_ != start_ && !traits::is_null_not_sparse(*it_))
            {
                // Skip sparse guys - they are just placeholders to keep spots allocated for
                // pointer stability
                // FIX: is_sparse is more bucket-dependent than linear probing wants it to be I think.
                // Falling back to is_empty since we already filtered out null meaning empty will ONLY
                // indicate sparse
                bool is_sparse = is_empty();
                //bool is_sparse = traits::is_sparse(*it_, n_);
                // Skip non-matching buckets as is the norm for linear probing
                bool is_foreign = parent_->index(traits::key(*it_)) != n_;

                // If not sparse or foreign, and not null - we are an active entry matching
                // the requested bucket, so don't consume
                if(!(is_sparse || is_foreign))  return true;

                bump_with_rollover();
            }

            // Reaching here means we either wrapped around - basically meaning end of the line -
            // or we reached a null, also the end of the line
            return false;
        }
#endif

        this_type& operator++()
        {
#if ISSUE_211_BRINGUP
            bump_with_rollover();
            skip_sparse_and_foreign_buckets();
#else
            ++it_;

            // skip over any sparse entries belonging to this bucket.  They are invisible
            // null entries for this iterator - specifically we are only revealing 'active' here
            for(; traits::is_sparse(*it_, n_) && it_ != parent_->container_.cend(); ++it_)   {}
#endif
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

    // represents invalid bucket
    // pointer and bucket
    template <class Pointer>
    using find_result = pair<Pointer, size_type>;

    // Iterate through all items in this particular bucket.  If we get a full
    // key match (not just hash match), return control_pointer and bucket of residence.
    // Otherwise, 'npos'
    template <class K>
    ESTD_CPP_CONSTEXPR(14) find_result<const_control_pointer> find_ll(const K& x) const
    {
        constexpr auto np = npos;

#if ESTD_UNORDERED_MAP_STRICT
        if(nullable::is_null(x))  return { container_.cend(), np };
#endif

        const size_type n = index(x);

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq_c(x, *it))    return { it.control(), n };

        return { container_.cend(), np };
    }

    template <class K>
    ESTD_CPP_CONSTEXPR(14) find_result<control_pointer> find_ll(const K& x)
    {
        // estd::pair behaves like it's taking the address of npos, resulting in
        // linker issues for c++11, c++14.  This works around that
        constexpr auto np = npos;

#if ESTD_UNORDERED_MAP_STRICT
        if(nullable::is_null(x))  return { container_.end(), np };
#endif
        const size_type n = index(x);

        for(local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq_c(x, *it))    return { it.control(), n };

        return { container_.end(), np };
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

        traits::swap(*pos, *start);
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
        return find_ll(key).second != npos;
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

#if NDEBUG
    using iterator = iterator_base<value_type>;
    using const_iterator = iterator_base<const value_type>;
    using local_iterator = local_iterator_base<pointer>;
    using const_local_iterator = local_iterator_base<const_pointer>;
#else
    // Not aliasing here purely for benefit of integrated debugging, reducing the template metadata spew

    struct iterator : iterator_base<value_type>
    {
        template <class ...Args>
        iterator(Args&&...args) : iterator_base<value_type>(std::forward<Args>(args)...) {}
    };

    struct const_iterator : iterator_base<const value_type>
    {
        template <class ...Args>
        const_iterator(Args&&...args) : iterator_base<const value_type>(std::forward<Args>(args)...) {}
    };

    using local_iterator = local_iterator_base<pointer>;
    using const_local_iterator = local_iterator_base<const_pointer>;
#endif

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

#if ISSUE_211_BRINGUP
    ESTD_CPP_CONSTEXPR(14) local_iterator begin(size_type n)
    {
        local_iterator it{ this, n, &container_[n] };
        it.skip_sparse_and_foreign_buckets();
        return it;
    }

    constexpr const_local_iterator begin(size_type n) const
    {
        return cbegin(n);
    }

    ESTD_CPP_CONSTEXPR(14) const_local_iterator cbegin(size_type n) const
    {
        const_local_iterator it{ this, n, &container_[n] };
        it.skip_sparse_and_foreign_buckets();
        return it;
    }

    constexpr end_local_iterator end(size_type n) const
    {
        return cend(n);
    }

    constexpr end_local_iterator cend(size_type n) const
    {
        return { &container_[n] };
    }
#else
    ESTD_CPP_CONSTEXPR(17) local_iterator begin(size_type n)
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
#endif
};

// UNTESTED for unodered_map
template <class Container, class Traits>
ESTD_CPP_CONSTEXPR(14) bool operator==(const unordered_base<Container, Traits>& lhs,
    const unordered_base<Container, Traits>& rhs)
{
    using traits = Traits;
    using it = typename unordered_base<Container, Traits>::const_iterator;

    // DEBT: Not optimal, since we have to iterate inside size() to make this determination
    if(lhs.size() != rhs.size())    return false;

    for(it l = lhs.begin(), r = rhs.begin(); l != lhs.cend(); ++l, ++r)
    {
        if(traits::key(*l) != traits::key(*r)) return false;
    }

    return true;
}


}

}

#include "../../macro/pop.h"
