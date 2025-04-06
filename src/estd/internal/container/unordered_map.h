#pragma once

#include "../../numeric.h"
#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"
#include "unordered/base.h"

namespace estd {

namespace internal {

template <class Container, class Traits>
class unordered_map : public unordered_base<Container, Traits>
{
    using this_type = unordered_map;
    using base_type = unordered_base<Container, Traits>;
    using base_type::index;
    using base_type::match;
    using base_type::is_null_or_sparse;
    using base_type::container_;
    using base_type::key_eq;
    using base_type::is_sparse;
    using base_type::skip_null;
    using base_type::cast;
    using base_type::insert_precheck;

#if UNIT_TESTING
public:
#endif

    // DEBT: Key SHOULD be value-initializable at this time.
    using control_type = typename base_type::control_type;
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;
    using typename base_type::end_local_iterator;

public:
    using base_type::bucket_depth;
    using typename base_type::key_type;
    using typename base_type::mapped_type;
    using typename base_type::nullable;

    using value_type = estd::pair<const key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using typename base_type::size_type;

    static constexpr size_type npos() { return numeric_limits<size_type>::max(); }

    using end_iterator = monostate;

    // Check that our casting wizardry doesn't get us into too much trouble
    static_assert(sizeof(typename base_type::meta) == sizeof(typename value_type::second_type),
            "size mismatch between meta and exposed value_type");
    static_assert(sizeof(control_type) == sizeof(value_type),
            "size mismatch between meta and exposed value_type");

    // pointer and bucket
    template <class Pointer>
    using find_result = pair<Pointer, size_type>;

    using insert_result = pair<control_pointer, bool>;

private:
    /// Checks for null but NOT sparse
    /// @param v
    /// @param n
    /// @return
    /// DEBT: Passing in size_type n seems optional since we have our gc flag
    template <class K, class T2>
    static constexpr bool is_null(const pair<K, T2>& v, size_type)
    {
        return is_null_or_spase(v) && cast_control(&v)->marked_for_gc == false;
    }

    /// Nulls out key
    /// @remark Does not run destructor
    static ESTD_CPP_CONSTEXPR(14) void set_null(control_pointer v)
    {
        nullable{}.set(&v->first);
    }


    // runs destructor + nulls out key
    static void destruct(control_pointer v)
    {
        set_null(v);
        v->second.mapped().~mapped_type();
    }

    static constexpr control_pointer cast_control(pointer pos)
    {
        return reinterpret_cast<control_pointer>(pos);
    }

    static constexpr const_control_pointer cast_control(const_pointer pos)
    {
        return reinterpret_cast<const_control_pointer>(pos);
    }

    // DEBT: Temporary as we transition container_ from value_type -> control_type
    constexpr pointer get_value(unsigned i) { return (pointer) &container_[i]; }
    ESTD_CPP_CONSTEXPR(14) pointer get_value_end() { return (pointer) container_.end(); }

    template <class It, class Parent>
    friend class base_type::iterator_base;


public:
    using iterator = typename base_type::template iterator_base<pointer, this_type>;
    using const_iterator = typename base_type::template iterator_base<const_pointer, this_type>;
    using local_iterator = typename base_type::template local_iterator_base<pointer>;
    using const_local_iterator = typename base_type::template local_iterator_base<const_pointer>;

private:

    // For insert/emplace operations specifically
    constexpr pair<iterator, bool> wrap_result(insert_result r) const
    {
        static_assert(is_same<typename insert_result::first_type, control_pointer>::value,
            "Failed sanity check: r.first must be castable to pointer");

        return { { this, (pointer)r.first }, r.second };
    }

    /// perform garbage collection on the bucket containing this active pos, namely moving
    /// pos if gc wishes it
    /// @param pos entry to possibly move
    /// @returns potentially moved 'pos'
    control_pointer gc_active_ll(control_pointer pos)
    {
        const key_type& key = pos->first;
        const size_type n = index(key);

        // look through other items in this bucket.  Not using local_iterator because he's
        // designed to skip over nulls, while we specifically are looking for those guys.
        // Also, we don't want to swap our active guy further down the bucket, only earlier
        for(control_pointer it = container_.begin(); it != container_.cend() && it < pos; ++it)
        {
            // if item is null (maybe) sparse
            if(is_null_or_sparse(*it))
            {
                control_pointer control = it;

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
                    it->swap(*pos);
                    return it;
                }
            }
        }

        return pos;
    }

public:
    ESTD_CPP_CONSTEXPR(14) unordered_map()
    {
        // DEBT: Feels clunky
        for(control_type& v : container_)   set_null(&v);
    }

    iterator begin()
    {
        return { this, skip_null(get_value(0)) };
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

    ESTD_CPP_CONSTEXPR(14) void clear()
    {
        for(control_type& v : container_)   destruct(&v);
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
        find_result<pointer> found = find_ll(key);

        if(found.second != npos()) return found.first->second;

        return try_emplace(key).first->second;
    }

    template <class ...Args>
    pair<iterator, bool> emplace(const key_type& key, Args&&...args)
    {
        const insert_result ret = insert_precheck(key, false);

        if(ret.second)
            new (ret.first) value_type(key, std::forward<Args>(args)...);

        return wrap_result(ret);
    }

    template <class ...Args1, class ...Args2>
    pair<iterator, bool> emplace(piecewise_construct_t,
        estd::tuple<Args1...>&& first_args,
        estd::tuple<Args2...>&& second_args,
        bool permit_duplicates = false)
    {
        const key_type& key = estd::get<0>(first_args);

        const insert_result ret = insert_precheck(key, permit_duplicates);

        if(ret.second)
            new (ret.first) value_type(piecewise_construct_t{},
                std::forward<estd::tuple<Args1...>>(first_args),
                std::forward<estd::tuple<Args2...>>(second_args));

        return wrap_result(ret);
    }

    // try_emplace only used right now since unlike regular emplace it can operate
    // without any parameter (aside from key)
    template <class K, class ...Args>
    pair<iterator, bool> try_emplace(const K& key, Args&&...args)
    {
        const insert_result ret = insert_precheck(key, false);

        // pair requires two parameters to construct, PLUS it's a const key.
        // fortunately, since we presume it's a trivial-ish type (no dtor)
        // we can brute force the key assignment
        if(ret.second)
        {
            // NOTE: Deviation from spec in that spec implies ->second gets value initialized,
            // where we do not do that.  Not 100% sure if that's what spec calls for though
            ESTD_CPP_IF_CONSTEXPR(sizeof...(Args) == 0)
                ret.first->first = key;
            else
                new (ret.first) value_type(piecewise_construct_t{},
                    forward_as_tuple(key),
                    forward_as_tuple(std::forward<Args>(args)...));
        }

        return wrap_result(ret);
    }

    pair<iterator, bool> insert(const_reference value, bool permit_duplicates = false)
    {
        const insert_result ret = insert_precheck(value.first, permit_duplicates);

        if(ret.second)
            // We've made it here without reaching the end or bonking into another bucket,
            // we're good to go
            new (ret.first) value_type(value);

        return wrap_result(ret);
    }

    template <class P>
    auto insert(P&& value, bool permit_duplicates = false) ->
        enable_if_t<is_constructible<value_type, P&&>::value, pair<iterator, bool>>
    {
        const insert_result ret = insert_precheck(value.first, permit_duplicates);

        if(ret.second)
            // We've made it here without reaching the end or bonking into another bucket,
            // we're good to go
            new (ret.first) value_type(std::forward<P>(value));

        return wrap_result(ret);
    }

    template <class K, class M>
    pair<iterator, bool> insert_or_assign(const K& k, M&& obj)
    {
        find_result<pointer> found = find_ll(k);

        if(found.second != npos())
        {
            reference p = found->first;
            new (&p.second) mapped_type(std::forward<M>(obj));
            return { { this, &p }, true };
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

    // NOTE: This works, but you'd prefer to avoid it and iterate yourself directly
    ESTD_CPP_CONSTEXPR(14) size_type bucket_size(size_type n) const
    {
        unsigned counter = 0;

        for(const_local_iterator it = cbegin(n); it != end(n); ++it)
            ++counter;

        return counter;
    }

    /*
    ESTD_CPP_CONSTEXPR(14) size_type size() const
    {
        return distance(begin(), cend());
    }   */

    template <class K>
    constexpr bool contains(const K& key) const
    {
        return find_ll(key).second != npos();
    }



    iterator gc_active(iterator pos)
    {
        return { this, (pointer) gc_active_ll(cast_control(pos.value())) };
    }

    // Demotes this sparse 'pos' to completely deleted 'null'
    void gc_sparse_ll(pointer pos)
    {
        //const key_type& key = pos->first;
        //const size_type n = index(key);

        assert(is_null_or_sparse(*pos));

        control_pointer control = cast_control(pos);

        //control->second.bucket = npos();
        control->second.marked_for_gc = false;
    }

    // Conforms to spec in that:
    // "References and iterators to the erased elements are invalidated.
    //  Other iterators and references are not invalidated. "
    void erase_ll(find_result<pointer> pos)
    {
        const size_type n = pos.second;

        destruct(cast_control(pos.first));

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
        erase_ll({ pos.value(), index(pos->first) });

        return { this, skip_null(pos.value() + 1) };
    }

    // deviates from std in that other iterators part of this bucket could be invalidated
    // DEBT: We do want to return 'iterator', it's just unclear from spec how that really works
    void erase_and_gc_ll(control_pointer pos)
    {
        auto start = pos;

        destruct(pos);

        ++pos;

        // No housekeeping if next guy is null (sparse is OK, but
        // not yet accounted for here)
        if(is_null_or_sparse(*pos)) return;

        // Quick-deduce our bucket#
        size_type n = start - container_.cbegin();
        // Find last one in bucket
        for(;n == index(pos->first) && pos < container_.cend(); ++pos) {}

        --pos;

        start->swap(*pos);
    }

    void erase_and_gc(iterator pos)
    {
        erase_and_gc_ll(cast_control(pos.value()));
    }

    void erase_and_gc(local_iterator pos)
    {
        erase_and_gc_ll(pos.control());
    }

    size_type erase(const key_type& key)
    {
        find_result<pointer> found = find_ll(key);

        if(found.first == npos()) return 0;

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
            if(key_eq()(key, it->first))
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
    ESTD_CPP_CONSTEXPR(14) find_result<const_pointer> find_ll(const K& x) const
    {
        const size_type n = index(x);

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq()(x, it->first))    return { it.cast(), n };

        return { cast(container_.cend()), npos() };
    }

    template <class K>
    ESTD_CPP_CONSTEXPR(14) find_result<pointer> find_ll(const K& x)
    {
        const size_type n = index(x);

        for(local_iterator it = begin(n); it != end(n); ++it)
            if(key_eq()(x, it->first))    return { it.cast(), n };

        return { cast(container_.end()), npos() };
    }

    template <class K>
    iterator find(const K& x)
    {
        return { this, find_ll(x).first };
    }

    template <class K>
    const_iterator find(const K& x) const
    {
        return { this, find_ll(x).first };
    }

    // Not ready yet because buckets don't preserve key order, so this gets tricky
    // Also, it's incongruous because elsewhere I read no duplicate keys allowed
    template <class K>
    pair<const_pointer, const_pointer> equal_range_exp(const K& x)
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
