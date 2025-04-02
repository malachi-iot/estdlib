#pragma once

#include "../../numeric.h"
#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"
#include "unordered/base.h"

namespace estd {

namespace internal {

template <unsigned N, class Key, class T, class Hash, class Nullable, class KeyEqual>
class unordered_map : public l1_unordered_base<N, unordered_traits<Key, Hash, KeyEqual, Nullable>>
{
    using base_type = l1_unordered_base<N, unordered_traits<Key, Hash, KeyEqual, Nullable>>;
    using base_type::index;
    using base_type::match;

#if UNIT_TESTING
public:
#endif

    union meta
    {
        byte storage[sizeof(T)];

        struct
        {
            // aka "sparse" - exists specifically to mark as deleted, but physically unmoved
            uint16_t marked_for_gc : 1;
            // which bucket this empty slot *used to* belong to
            uint16_t bucket : 6;
        };

        operator T& () { return * (T*) storage; }
        constexpr operator const T& () const { return * (T*) storage; }

        T& mapped() { return * (T*) storage; }
    };

    // DEBT: Key SHOULD be value-initializable at this time.
    // TODO: eventually container is populated by control_type, not value_type
    using control_type = estd::pair<Key, meta>;
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;

public:
    using base_type::bucket_depth;
    using typename base_type::key_type;
    using mapped_type = T;

    using value_type = estd::pair<const Key, T>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using typename base_type::hasher;
    using typename base_type::size_type;

    static constexpr size_type npos() { return numeric_limits<size_type>::max(); }

    struct end_local_iterator
    {
        const_iterator it_;
    };

    using end_iterator = monostate;

    // Check that our casting wizardry doesn't get us into too much trouble
    static_assert(sizeof(meta) == sizeof(typename value_type::second_type),
            "size mismatch between meta and exposed value_type");
    static_assert(sizeof(control_type) == sizeof(value_type),
            "size mismatch between meta and exposed value_type");

    //using local_iterator = iterator;
    //using const_local_iterator = const_iterator;

    // pointer and bucket
    template <class Pointer>
    using find_result = pair<Pointer, size_type>;

    using insert_result = pair<iterator, bool>;

private:
    // DEBT: casting to control a bummer
    static constexpr void swap(pointer lhs, pointer rhs)
    {
        cast_control(lhs)->swap(*cast_control(rhs));
    }

    /// @brief Checks for null OR sparse
    /// @param v
    /// @return
    static constexpr bool is_null_or_spase(const_reference v)
    {
        return Nullable{}.is_null(v.first);
    }

    /// Checks for null but NOT sparse
    /// @param v
    /// @param n
    /// @return
    /// DEBT: Passing in size_type n seems optional since we have our gc flag
    static constexpr bool is_null(const_reference v, size_type)
    {
        return is_null_or_spase(v) && cast_control(&v)->marked_for_gc == false;
    }

    /// Determines if this ref is sparse - bucket must match also
    /// @param v
    /// @param n bucket#
    /// @return
    static constexpr bool is_sparse(const_reference v, size_type n)
    {
        const_control_pointer ctl = cast_control(&v);

        return is_null_or_spase(v) &&
            ctl->second.marked_for_gc &&
            ctl->second.bucket == n;
    }

    /// Nulls out key
    /// @param v
    /// @remark Does not run destructor
    static void set_null(pointer v)
    {
        // DEBT: Control-casting annoying, but a slight improvement over const_cast
        Nullable{}.set(&cast_control(v)->first);
    }

    // runs destructor + nulls out key
    static void destruct(pointer v)
    {
        set_null(v);
        v->second.~mapped_type();
    }

    static constexpr control_pointer cast_control(pointer pos)
    {
        return reinterpret_cast<control_pointer>(pos);
    }

    static constexpr const_control_pointer cast_control(const_pointer pos)
    {
        return reinterpret_cast<const_control_pointer>(pos);
    }

    uninitialized_array<value_type, N> container_;

    // It must be pointer or const_pointer
    template <class It>
    ESTD_CPP_CONSTEXPR(14) It skip_null(It it) const
    {
        for(; is_null_or_spase(*it) && it != container_.cend(); ++it)   {}

        return it;
    }

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

            it_ = parent_->skip_null(it_);

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
            if(is_null_or_spase(*it_)) return true;

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
            if(is_null_or_spase(*it_)) return false;

            // if n_ matches current key hash, we haven't yet reached the
            // end of this bucket
            return n_ == index(it_->first);
        }

        this_type& operator++()
        {
            ++it_;

            // skip over any sparse entries belonging to this bucket.  They are invisible
            // null entries for this iterator
            for(; is_sparse(*it_, n_) && it_ != parent_.container_.cend(); ++it_)   {}

            return *this;
        }

        this_type operator++(int)
        {
            operator++();

            return { n_, it_ - 1 };
        }
    };

    template <class K>
    insert_result insert_precheck(const K& key, bool permit_duplicates)
    {
        const size_type n = index(key);

        // linear probing

        iterator it = &container_[n];
        // Move over occupied spots.  Sparse also counts as occupied
        // DEBT: optimize is_null/is_sparse together
        for(;is_null_or_spase(*it) == false || is_sparse(*it, n); ++it)
        {
            // if we get to the complete end, that's a fail
            // if we've moved to the next bucket, that's also a fail
            if(it == container_.cend() || index(it->first) != n)
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

    ESTD_CPP_CONSTEXPR(14) unordered_map()
    {
        // DEBT: Feels clunky
        for(reference v : container_)   set_null(&v);
    }

    // NOTE: Not sure if end/cend represents end of raw container or end of active, useful
    // buckets but I think it's the former
    // FIX: https://en.cppreference.com/w/cpp/container/unordered_map/clear
    // finally tells us that these begin/ends are supposed to filter by not-nulled
    constexpr const_iterator cend_old() const { return container_.cend(); }

    iterator_base<pointer> begin()
    {
        return { this, skip_null(container_.begin()) };
    }

    constexpr iterator_base<const_pointer> begin() const
    {
        return { this, skip_null(container_.cbegin()) };
    }

    // DEBT: as above, eventually displaces things
    // DEBT: can probably use a hard type like end_iterator (optimization) though
    // that does double down on carrying parent* around
    constexpr iterator_base<const_pointer> end() const
    {
        return { this, container_.cend() };
    }

    ESTD_CPP_CONSTEXPR(14) void clear()
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
        pointer found = find_ll(key).first;

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
        const pair<iterator, bool> ret = insert_precheck(key, false);

        // pair requires two parameters to construct, PLUS it's a const key.
        // fortunately, since we presume it's a trivial-ish type (no dtor)
        // we can brute force the key assignment
        if(ret.second)
        {
            // NOTE: Deviation from spec in that spec implies ->second gets value initialized,
            // where we do not do that.  Not 100% sure if that's what spec calls for though
            ESTD_CPP_IF_CONSTEXPR(sizeof...(Args) == 0)
                cast_control(ret.first)->first = key;
            else
                new (ret.first) value_type(piecewise_construct_t{},
                    forward_as_tuple(key),
                    forward_as_tuple(std::forward<Args>(args)...));
        }

        return ret;
    }

    pair<iterator, bool> insert(const_reference value, bool permit_duplicates = false)
    {
        const pair<iterator, bool> ret = insert_precheck(value.first, permit_duplicates);

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
        return distance(
            begin(),
            {this, container_.end()});
    }

    template <class K>
    constexpr bool contains(const K& key) const
    {
        return find_ll(key).first != container_.cend();
    }

    /// perform garbage collection on the bucket containing this active pos, namely moving
    /// pos if gc wishes it
    /// @param pos entry to possibly move
    /// @returns potentially moved 'pos'
    pointer gc_active_ll(pointer pos)
    {
        const key_type& key = pos->first;
        const size_type n = index(key);

        // look through other items in this bucket.  Not using local_iterator because he's
        // designed to skip over nulls, while we specifically are looking for those guys.
        // Also, we don't want to swap our active guy further down the bucket, only earlier
        for(pointer it = &container_[n]; it != container_.cend() && it < pos; ++it)
        {
            // if item is null (maybe) sparse
            if(is_null_or_spase(*it))
            {
                control_pointer control = cast_control(it);

                // if sparse, it's not a swap candidate
                if(control->second.marked_for_gc)
                {
                    // make sure we're in the same bucket
                    // moving out of the bucket terminates the GC operation, no null slot found
                    if(control->second.bucket != n) return pos;
                }
                // if regular null (not sparse) and we are physically before active
                // item 'pos', do a swap and exit
                else
                {
                    swap(it, pos);
                    return it;
                }
            }
        }

        return pos;
    }

    // Demotes this sparse 'pos' to completely deleted 'null'
    void gc_sparse_ll(pointer pos)
    {
        //const key_type& key = pos->first;
        //const size_type n = index(key);

        assert(is_null_or_spase(*pos));

        control_pointer control = cast_control(pos);

        control->second.bucket = npos();
        control->second.marked_for_gc = false;
    }

    // Conforms to spec in that:
    // "References and iterators to the erased elements are invalidated.
    //  Other iterators and references are not invalidated. "
    void erase_ll(find_result<pointer> pos)
    {
        const size_type n = pos.second;

        destruct(pos.first);

        // "mark and sweep" erase rather than erase (and swap) immediately in place.
        // More inline with spec, namely doesn't disrupt other iterators
        control_pointer control = cast_control(pos.first);

        control->second.marked_for_gc = 1;
        control->second.bucket = n;
    }

    // https://en.cppreference.com/w/cpp/container/unordered_map/erase
    // NOTE: example implies internal ordering of unordered_map is predictable, which
    // on one hand feels reasonable, but on the other seems to conflict with the notion
    // that we are officially unordered.
    iterator erase(iterator pos)
    {
        erase_ll({ pos, index(pos->first) });

        return skip_null(pos + 1);
    }

    // deviates from std in that other iterators part of this bucket could be invalidated
    // DEBT: We do want to return 'iterator', it's just unclear from spec how that really works
    void erase_and_gc_ll(pointer pos)
    {
        pointer start = pos;

        destruct(pos);

        ++pos;

        // No housekeeping if next guy is null (sparse is OK, but
        // not yet accounted for here)
        if(is_null_or_spase(*pos)) return;

        // Quick-deduce our bucket#
        size_type n = start - container_.cbegin();
        // Find last one in bucket
        for(;n == index(pos->first) && pos < container_.cend(); ++pos) {}

        --pos;

        swap(start, pos);
    }

    void erase_and_gc(iterator pos)
    {
        erase_and_gc_ll(pos);
    }

    size_type erase(const key_type& key)
    {
        find_result<pointer> found = find_ll(key);

        if(found.first == container_.cend()) return 0;

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
    find_result<const_pointer> find_ll(const K& x) const
    {
        const size_type n = index(x);

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            if(KeyEqual{}(x, it->first))    return { it.it_, n };

        return { container_.cend(), npos() };
    }

    template <class K>
    find_result<pointer> find_ll(const K& x)
    {
        const size_type n = index(x);

        for(local_iterator it = begin(n); it != end(n); ++it)
            if(KeyEqual{}(x, it->first))    return { it.it_, n };

        return { container_.end(), npos() };
    }

    template <class K>
    iterator find(const K& x)
    {
        return find_ll(x).first;
    }

    template <class K>
    const_iterator find(const K& x) const
    {
        return find_ll(x).first;
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
