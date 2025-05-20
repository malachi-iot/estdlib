#pragma once

#include "../../numeric.h"
#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"
#include "unordered/base.h"

#include "../macro/push.h"

namespace estd {

namespace internal {

template <class Container, class Traits>
class unordered_map : public unordered_base<Container, Traits>
{
    using this_type = unordered_map;
    using base_type = unordered_base<Container, Traits>;
    using base_type::index;
    using base_type::match;
    using base_type::is_null_not_sparse;
    using base_type::is_null_or_sparse;
    using base_type::container_;
    using base_type::is_sparse;
    using base_type::skip_null;
    using base_type::cast;
    using base_type::insert_precheck;
    using traits = Traits;

    template <class Pointer>
    using find_result = typename base_type::template find_result<Pointer>;

#if UNIT_TESTING
public:
#endif

    // DEBT: Key SHOULD be value-initializable at this time.
    using control_type = typename base_type::control_type;
    using control_pointer = control_type*;
    using const_control_pointer = const control_type*;
    using typename base_type::end_local_iterator;
    using typename base_type::insert_result;

public:
    using base_type::find_ll;
    using base_type::bucket_depth;
    using base_type::begin;
    using base_type::end;
    using base_type::key_eq;
    using base_type::set_null;
    using base_type::destruct;
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
    static_assert(sizeof(control_type::first) == sizeof(value_type::first),
        "size mismatch between key of control_type and value_type");
    static_assert(offsetof(control_type, second) == offsetof(value_type, second),
            "mapped_type position mismatch between control_type and value_type");
    static_assert(sizeof(control_type) == sizeof(value_type),
            "size mismatch between meta and exposed value_type");

private:
    /// Checks for null but NOT sparse
    /// @param v
    /// @param n
    /// @return
    /// DEBT: Passing in size_type n seems optional since we have our gc flag
    template <class K, class T2>
    static constexpr bool is_null(const pair<K, T2>& v, size_type)
    {
        return is_null_or_sparse(v) && cast_control(&v.second)->marked_for_gc == false;
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

public:
    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using typename base_type::local_iterator;
    using typename base_type::const_local_iterator;

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
        for(control_pointer it = container_.begin() + n; it != container_.cend() && it < pos; ++it)
        {
            // if item is null (maybe) sparse
            if(is_null_or_sparse(*it))
            {
                control_pointer control = it;

                // if sparse, double check we're in the same bucket still
                if(control->second.marked_for_gc)
                {
                    // moving out of the bucket terminates the GC operation, no null slot found
                    // NOTE: we could make an extended mode which just reaches on forever, linear probing doesn't
                    // prohibit that at all - it's just crossing into a new bucket now we have to do some extra checks.
                    // a prune_sparse_ll may make that easier
                    if(control->second.bucket != n) return pos;
                }

                it->swap(*pos);
                return it;
            }
        }

        return pos;
    }

#if UNIT_TESTING
public:
#endif
    //
    ///
    /// @brief prune_sparse_ll null out trailing sparse entries, moving backward
    /// @param start DEBT: rename - this is end point
    /// @param pos where to begin nulling from
    /// @param n
    ///
    void prune_sparse_ll(control_pointer start, control_pointer pos, size_type n)
    {
        --start;

        for(; is_sparse(*pos, n) && pos != start; --pos)
            gc_sparse_ll(pos);
    }

    // for every trailing marked_for_gc (doesn't have an active slot after him) turn to null
    // UNTESTED
    void prune_sparse_ll(control_pointer pos)
    {
        control_pointer start = pos;
        const size_t n = pos->second.bucket;

        // get to end of identified bucket, skipping past sparse and active bucket items
        // DEBT: Be careful of index(0)
        for(;(is_sparse(*pos, n) || index(pos->first) == n) && pos != container_.cend(); ++pos)
        {
        }

        if(pos == start) return;        // found null right away, which is end of bucket

        prune_sparse_ll(start, --pos, n);
    }

public:
    ESTD_CPP_CONSTEXPR(14) void clear()
    {
        for(control_type& v : container_)   destruct(&v);
    }

    mapped_type& operator[](const key_type& key)
    {
        find_result<control_pointer> found = find_ll(key);

        if(found.second != npos()) return found.first->second.mapped();

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
                    estd::forward_as_tuple(key),
                    estd::forward_as_tuple(std::forward<Args>(args)...));
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

    // NOTE: This works, but you'd prefer to avoid it and iterate yourself directly
    ESTD_CPP_CONSTEXPR(14) size_type bucket_size(size_type n) const
    {
        unsigned counter = 0;

        for(const_local_iterator it = begin(n); it != end(n); ++it)
            ++counter;

        return counter;
    }

    iterator gc_active(iterator pos)
    {
        return { this, (pointer) gc_active_ll(cast_control(pos.value())) };
    }

    void gc_sparse_ll(control_pointer control)
    {
        //const key_type& key = pos->first;
        //const size_type n = index(key);

        assert(is_null_or_sparse(*control));

        //control->second.bucket = npos();
        control->second.marked_for_gc = false;

        //prune_sparse_ll(control_pointer);
    }

    // Demotes this sparse 'pos' to completely deleted 'null'
    void gc_sparse_ll(pointer pos)
    {
        gc_sparse_ll(cast_control(pos));
    }

    // Conforms to spec in that:
    // "References and iterators to the erased elements are invalidated.
    //  Other iterators and references are not invalidated. "
    void erase_ll(find_result<control_pointer> pos, bool auto_prune = true)
    {
        const size_type n = pos.second;
        control_pointer control = pos.first;

        destruct(control);

        const_control_pointer next = control + 1;

        // If no further bucket entries, do prune
        // that means:
        // - if we're at the end
        // - if next entry is an active item in a different bucket
        // - if next entry is a null entry
        // then we are clear to null out trailing sparse entries
        if(auto_prune && (next == container_.cend() || is_null_not_sparse(*next)))
        {
            control_pointer start = container_.begin() + n;
            prune_sparse_ll(start, control, n);
        }
        else
        {
            // "mark and sweep" erase rather than erase (and swap) immediately in place.
            // More inline with spec, namely doesn't disrupt other iterators
            control->second.marked_for_gc = 1;
            control->second.bucket = n;
        }
    }

    // equivelant to erase with iterator, but merely takes direct value_type*
    void erase_ll(pointer pos)
    {
        erase_ll({ cast_control(pos), index(traits::key(*pos)) });
    }

    // https://en.cppreference.com/w/cpp/container/unordered_map/erase
    // NOTE: example implies internal ordering of unordered_map is predictable, which
    // on one hand feels reasonable, but on the other seems to conflict with the notion
    // that we are officially unordered.
    iterator erase(iterator pos)
    {
        erase_ll(pos.value());

        return { this, skip_null(pos.value() + 1) };
    }

    void erase_and_gc(iterator pos)
    {
        base_type::erase_and_gc_ll(cast_control(pos.value()));
    }

    void erase_and_gc(local_iterator pos)
    {
        base_type::erase_and_gc_ll(pos.control());
    }

    size_type erase(const key_type& key)
    {
        find_result<control_pointer> found = find_ll(key);

        if(found.second == npos()) return 0;

        erase_ll(found);
        return 1;
    }

    template <class K>
    iterator find(const K& x)
    {
        return { this, cast(find_ll(x).first) };
    }

    template <class K>
    const_iterator find(const K& x) const
    {
        return { this, cast(find_ll(x).first) };
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

#include "../macro/pop.h"
