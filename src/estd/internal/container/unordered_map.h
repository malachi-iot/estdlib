#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"
#include "unordered/base.h"

namespace estd {

namespace internal {

// DEBT: Hard-wired to layer1 stlye

template <
    unsigned sz,
    class Key,
    class T,
    class Hash = hash<Key>,
    class Nullable = nullable_traits<Key>,
    class KeyEqual = equal_to<Key> >
class unordered_map;

template <unsigned N, class Key, class T, class Hash, class Nullable, class KeyEqual>
class unordered_map : public l1_unordered_base<N, unordered_traits<Key, Hash, KeyEqual>>
{
    using base_type = l1_unordered_base<N, unordered_traits<Key, Hash, KeyEqual>>;
    using base_type::index;
    using base_type::match;

    union meta
    {
        byte storage[sizeof(T)];

        struct
        {
            uint16_t marked_for_gc : 1;
            // which bucket this empty slot *used to* belong to
            uint16_t bucket : 6;
        };

        operator T& () { return * (T*) storage; }
        constexpr operator const T& () const { return * (T*) storage; }

        T& mapped() { return * (T*) storage; }
    };

    // DEBT: Key SHOULD be value-initializable at this time.
    using control_type = estd::pair<Key, meta>;
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;

public:
    using base_type::bucket_depth;
    using typename base_type::key_type;
    using mapped_type = T;

    // EXPERIMENTAL
    union key_wrapper
    {
        const key_type key;
        key_type nulled;

        operator const key_type&() { return key; }
    };

    using value_type = estd::pair<const Key, T>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using typename base_type::hasher;
    using typename base_type::size_type;

    struct end_local_iterator
    {
        const_iterator it_;
    };

    // DEBT: This sucks, a glorified const_cast
    using cheater_iterator = estd::pair<Key, T>*;
    using end_iterator = monostate;

    // Check that our casting wizardry doesn't get us into too much trouble
    static_assert(sizeof(meta) == sizeof(typename value_type::second_type), "");
    static_assert(sizeof(control_type) == sizeof(value_type), "");

    //using local_iterator = iterator;
    //using const_local_iterator = const_iterator;

private:
    // DEBT: casting away Key const in this crude manner
    static constexpr void swap(iterator lhs, iterator rhs)
    {
        reinterpret_cast<cheater_iterator>(lhs)->swap(
            *reinterpret_cast<cheater_iterator>(rhs));
    }

    static constexpr bool is_null(const_reference v)
    {
        return Nullable{}.is_null(v.first);
    }

    /// Checks for null but NOT sparse
    /// @param v
    /// @param n
    /// @return
    static constexpr bool is_null(const_reference v, size_type n)
    {
        auto ctl = (const_control_pointer) &v;

        return Nullable{}.is_null(v.first);
    }

    ///
    /// @param v
    /// @param n bucket#
    /// @return
    static constexpr bool is_sparse(const_reference v, size_type n)
    {
        auto ctl = (const_control_pointer) &v;

        return is_null(v) &&
            ctl->second.marked_for_gc &&
            ctl->second.bucket == n;
    }

    ///
    /// @param v
    /// @remark Does not run destructor
    static void set_null(reference v)
    {
        // DEBT: I hate const_cast'ing
        Nullable{}.set(const_cast<key_type*>(&v.first));
    }

    static void destruct(pointer v)
    {
        set_null(*v);
        v->second.~mapped_type();
    }

    uninitialized_array<value_type, N> container_;

    // semi-smart, can skip null spots
    template <class It>
    class iterator_base
    {
        using parent_type = unordered_map;
        using this_type = iterator_base;

        const parent_type* parent_;
        It it_;

        template <class OtherIt>
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

            // skip over null entries
            for(; is_null(*it_) && it_ != parent_->cend(); ++it_)   {}

            return *this;
        }

        constexpr const_reference operator*() const { return *it_; }

        constexpr const_pointer operator->() const { return it_; }

        pointer operator->() { return it_; }

        template <class OtherIt>
        constexpr bool operator==(const iterator_base<OtherIt>& other) const
        {
            return it_ == other.it_;
        }

        template <class OtherIt>
        constexpr bool operator!=(const iterator_base<OtherIt>& other) const
        {
            return it_ != other.it_;
        }
    };

    template <class LocalIt>
    struct local_iterator_base
    {
        using parent_type = unordered_map;
        using this_type = local_iterator_base;

        const parent_type& parent_;

        // bucket designator
        const size_type n_;

        LocalIt it_;

        constexpr const_reference operator*() const { return *it_; }

        constexpr const_pointer operator->() const { return it_; }

        operator LocalIt&() { return it_; }

        constexpr bool operator==(end_local_iterator it) const
        {
            // If we reach end of entire set, indicate we are at the end
            if(it_ == it.it_)   return true;

            // If we reach a null slot, then that's the end of the bucket
            if(is_null(*it_)) return true;

            // if n_ doesn't match current key hash, we have reached the end
            // of this bucket
            return n_ != index(it_->first);
        }

        constexpr bool operator!=(end_local_iterator it) const
        {
            // If we reach end of entire set, indicate we are NOT NOT at the end
            if(it_ == it.it_)   return false;

            // If we reach a null slot, that's the end of the bucket - so we fail
            // to assert it's not the end (return false)
            if(is_null(*it_)) return false;

            // if n_ matches current key hash, we haven't yet reached the
            // end of this bucket
            return n_ == index(it_->first);
        }

        this_type& operator++()
        {
            ++it_;

            for(; is_sparse(*it_, n_) && it_ != parent_.cend(); ++it_)   {}

            return *this;
        }

        this_type operator++(int)
        {
            operator++();

            return { n_, it_ - 1 };
        }
    };

    template <class K>
    pair<iterator, bool> insert_precheck(const K& key, bool permit_duplicates)
    {
        const size_type n = index(key);

        // linear probing

        iterator it = &container_[n];
        // Move over occupied spots.  Sparse also counts as occupied
        // DEBT: optimize is_null/is_sparse together
        for(;is_null(*it) == false || is_sparse(*it, n); ++it)
        {
            // if we get to the complete end, that's a fail
            // if we've moved to the next bucket, that's also a fail
            if(it == cend() || index(it->first) != n)
                return { nullptr, false };
            else if(!permit_duplicates)
            {
                if(KeyEqual{}(key, it->first))
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


public:
    using local_iterator = local_iterator_base<pointer>;
    using const_local_iterator = local_iterator_base<const_pointer>;

    unordered_map()
    {
        // DEBT: Feels clunky
        for(reference v : container_)   set_null(v);
    }

    // NOTE: Not sure if end/cend represents end of raw container or end of active, useful
    // buckets but I think it's the former
    // FIX: https://en.cppreference.com/w/cpp/container/unordered_map/clear
    // finally tells us that these begin/ends are supposed to filter by not-nulled
    constexpr iterator end() { return container_.end(); }
    constexpr const_iterator cend() const { return container_.cend(); }
    constexpr const_iterator cbegin() const { return container_.begin(); }

    // DEBT: eventually this displaces current end/begin
    iterator_base<pointer> begin2()
    {
        pointer p = container_.begin();
        for(; is_null(*p) && p != container_.cend(); ++p)   {}

        return { this, p };
    }

    // DEBT: as above, eventually displaces things
    // DEBT: can probably use a hard type like end_iterator (optimization) though
    // that does double down on carrying parent* around
    constexpr iterator_base<const_pointer> end2() const
    {
        return { this, container_.cend() };
    }

    void clear()
    {
        for(reference v : container_)   destruct(&v);
    }

    // DEBT: May be a deviation since our buckets are a little more fluid, but I think
    // it conforms to spec
    template <class K>
    constexpr size_type bucket(const K& key) const
    {
        return index(key);
    }

    mapped_type& operator[](const key_type& key)
    {
        // DEBT: Hate const_cast
        auto found = const_cast<pointer>(find_ll(key));

        if(found != container_.cend()) return found->second;

        return try_emplace(key).first->second;
    }

    template <class ...Args>
    pair<iterator, bool> emplace(const key_type& key, Args&&...args)
    {
        pair<iterator, bool> ret = insert_precheck(key, false);

        if(ret.second)
            new (ret.first) value_type(key, std::forward<Args>(args)...);

        return ret;
    }

    template <class ...Args1, class ...Args2>
    pair<iterator, bool> emplace(piecewise_construct_t,
        estd::tuple<Args1...>&& first_args,
        estd::tuple<Args2...>&& second_args,
        bool permit_duplicates = false)
    {
        const key_type& key = estd::get<0>(first_args);

        pair<iterator, bool> ret = insert_precheck(key, permit_duplicates);

        if(ret.second)
            new (ret.first) value_type(piecewise_construct_t{},
                std::forward<estd::tuple<Args1...>>(first_args),
                std::forward<estd::tuple<Args2...>>(second_args));

        return ret;
    }

    // try_emplace only used right now since unlike regular emplace it can operate
    // without any parameter (aside from key)
    template <class K, class ...Args>
    pair<iterator, bool> try_emplace(const K& key, Args&&...args)
    {
        pair<iterator, bool> ret = insert_precheck(key, false);

        // pair requires two parameters to construct, PLUS it's a const key.
        // fortunately, since we presume it's a trivial-ish type (no dtor)
        // we can brute force the key assignment
        if(ret.second)
        {
            // NOTE: Deviation from spec in that spec implies ->second gets value initialized,
            // where we do not do that.  Not 100% sure if that's what spec calls for though
            if constexpr(sizeof...(Args) == 0)
                reinterpret_cast<cheater_iterator>(ret.first)->first = key;
            else
                new (ret.first) value_type(piecewise_construct_t{},
                    forward_as_tuple(key),
                    forward_as_tuple(std::forward<Args>(args)...));
        }

        return ret;
    }

    pair<iterator, bool> insert(const_reference value, bool permit_duplicates = false)
    {
        pair<iterator, bool> ret = insert_precheck(value.first, permit_duplicates);

        if(ret.second)
            // We've made it here without reaching the end or bonking into another bucket,
            // we're good to go
            new (ret.first) value_type(value);

        return ret;
    }

    template <class P>
    auto insert(P&& value, bool permit_duplicates = false) ->
        enable_if_t<is_constructible<value_type, P&&>::value, pair<iterator, bool>>
    {
        pair<iterator, bool> ret = insert_precheck(value.first, permit_duplicates);

        if(ret.second)
            // We've made it here without reaching the end or bonking into another bucket,
            // we're good to go
            new (ret.first) value_type(std::forward<P>(value));

        return ret;
    }

    template <class K, class M>
    pair<iterator, bool> insert_or_assign(const K& k, M&& obj)
    {
        iterator found = find(k);

        if(found != container_.cend())
        {
            new (&found->second) mapped_type(std::forward<M>(obj));
            return { found, true };
        }
        else
        {
            // DEBT: Would prefer an emplace here, but it's not smart enough to reliably
            // sort out class K
            return insert({k, std::forward<M>(obj)});
        }
    }

    local_iterator begin(size_type n)
    {
        return { *this, n, &container_[n] };
    }

    const_local_iterator begin(size_type n) const
    {
        return { *this, n, &container_[n] };
    }

    const_local_iterator cbegin(size_type n) const
    {
        return { *this, n, &container_[n] };
    }

    constexpr end_local_iterator end(size_type) const
    {
        return { container_.cend() };
    }

    constexpr end_local_iterator cend(size_type) const
    {
        return { container_.cend() };
    }

    // NOTE: This works, but you'd prefer to avoid it and iterate yourself directly
    size_type bucket_size(size_type n) const
    {
        unsigned counter = 0;

        for(const_local_iterator it = cbegin(n); it != end(n); ++it)
            ++counter;

        return counter;
    }

    size_type size() const
    {
        iterator_base<const_pointer> it(*this, container_.cbegin());

        // TODO: Use estd::accumulate

        return {};
    }

    template <class K>
    constexpr bool contains(const K& key) const
    {
        return find_ll(key) != container_.cend();
    }

    /// perform garbage collection on the bucket containing this pos, namely swapping this pos
    /// if gc wishes it
    /// @param pos entry to possibly move
    /// @returns potentially moved 'pos'
    pointer gc_ll(pointer pos)
    {
        const key_type& key = pos->first;
        const size_type n = index(key);

        for(local_iterator it = begin(n); it != cend(n); ++it)
        {
            if(is_null(*it) && it < pos)
            {
                swap(it, pos);
                return it;
            }
        }

        return pos;
    }

    // Not operational yet
    void erase_ll(pointer pos)
    {
        const size_type n = index(pos->first);

        destruct(pos);

        // "mark and sweep" erase rather than erase (and swap) immediately in place.
        // More inline with spec, namely doesn't disrupt other iterators
        auto control = reinterpret_cast<control_pointer>(pos);

        // NOTE: Doesn't do anything yet
        control->second.marked_for_gc = 1;
        control->second.bucket = n;
    }

    void erase(iterator pos) { erase_ll(pos); }

    // deviates from std in that other iterators part of this bucket could be invalidated
    // DEBT: We do want to return 'iterator', it's just unclear from spec how that really works
    void erase_and_gc_ll(pointer pos)
    {
        pointer start = pos;

        destruct(pos);

        ++pos;

        // No housekeeping if next guy is null
        if(is_null(*pos)) return;

        // Quick-deduce our bucket#
        size_type n = start - cbegin();
        // Find last one in bucket
        for(;n == index(pos->first) && pos < cend(); ++pos) {}

        --pos;

        swap(start, pos);
    }

    void erase_and_gc(iterator pos)
    {
        erase_and_gc_ll(pos);
    }

    size_type erase(const key_type& key)
    {
        pointer found = find_ll(key);

        if(found == container_.cend()) return 0;

        erase_ll(found);
        return 1;
    }

    // In fact, works for find too but it seems to make things more complicated,
    // not more tidy
    template <class K, class F, class R = monostate>
    R&& bucket_foreach(const K& key, F&& f, R&& r = monostate{}) const
    {
        const size_type n = bucket(key);
        for(const_local_iterator it = begin(n); it != end(n); ++it)
        {
            if(KeyEqual{}(key, it->first))
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

    template <class K>
    size_type count(const K& x) const
    {
        unsigned counter = 0;
        bucket_foreach(x, [&](const_local_iterator) { ++counter; });
        return counter;
    }

    template <class K>
    const_pointer find_ll(const K& x) const
    {
        const size_type n = index(x);

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            if(KeyEqual{}(x, it->first))    return it;

        return container_.cend();
    }

    template <class K>
    pointer find(const K& x)
    {
        // DEBT: I hate const_cast
        return const_cast<pointer>(find_ll(x));
    }

    template <class K>
    const_iterator find(const K& x) const
    {
        return find_ll(x);
    }

    // Not ready yet because buckets don't preserve key order, so this gets tricky
    // Also, it's incongruous because elsewhere I read no duplicate keys allowed
    template <class K>
    pair<const_iterator, const_iterator> equal_range_exp(const K& x)
    {
        const size_type n = index(x);
        const_local_iterator start = cbegin(n);
        const_local_iterator it = start;

        for(; it != end(n); ++it)
        {
        }

        return { start, it };
    }
};

}

}
