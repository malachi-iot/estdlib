#pragma once

#include "../../numeric.h"
#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"
#include "../raw/memory.h"
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
    using base_type::container_;
    using base_type::is_tombstone;
    //using base_type::skip_empty_old;
    using base_type::skip_empty_new;
    //using base_type::skip_empty;
    using base_type::cast;
    using base_type::insert_precheck;
    using traits = Traits;

#if UNIT_TESTING
public:
#endif

    // DEBT: Key SHOULD be value-initializable at this time.
    using typename base_type::control_type;
    using typename base_type::control_pointer;
    using typename base_type::const_control_pointer;
    using typename base_type::end_local_iterator;
    using typename base_type::insert_result;

public:
    using base_type::bump;
    using base_type::rbump;
    using base_type::cast_control;
    using base_type::is_null_not_sparse;
    using base_type::is_empty;
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
    using typename base_type::size_type;

    template <class Pointer>
    using find_result = typename base_type::template find_result<Pointer>;

    ESTD_CPP_STD_VALUE_TYPE(typename traits::value_type)

    static constexpr size_type npos() { return numeric_limits<size_type>::max(); }

    using end_iterator = monostate;

private:
    // pointer and control_pointer overlap.  See unordered_base for breakdown

public:
    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using typename base_type::local_iterator;
    using typename base_type::const_local_iterator;

private:
    /// Destroys key and value, and nulls out key (but leaves control portion in undefined state)
    ESTD_CPP_CONSTEXPR(14) static void destruct(control_pointer v)
    {
        base_type::destruct_ll(traits::key(*v));
        base_type::destruct(v);
    }

    // For insert/emplace operations specifically
    constexpr pair<iterator, bool> wrap_result(insert_result r) const
    {
        static_assert(is_same<typename insert_result::first_type, control_pointer>::value,
            "Failed sanity check: r.first must be castable to pointer");

        return { { this, (pointer)r.first }, r.second };
    }

    /// perform garbage collection on the bucket containing this active pos, namely moving
    /// pos to first empty slot starting from 'begin' in bucket.
    /// @param pos entry to possibly move
    /// @param begin position to start from, useful for GC'ing multiple entries
    /// @param n bucket of interest.  Must be bucket in which 'pos' resides
    /// @returns potentially moved 'pos'
    /// @remarks we pass in 'n' as an optimimzation to avoid double-calculating hash
    control_pointer gc_active_ll(
        control_pointer pos,
        control_pointer begin, const size_type n)
    {
        using modes = unordered_map_control_enum::modes;
        const_control_pointer begin_n = container_.cbegin() + n;

        assert(index(traits::key(*pos)) == n);        // Verify pos really is part of 'n'

        for(control_pointer it = begin; it != pos;)
        {
            // if item is null or tombstoned
            if(is_empty(*it))
            {
                // true null = end of bucket.  If we see true null, that means
                // data was corrupted or 'pos' is invalid, since 'pos' MUST exist
                // in a linear-probe-discoverable way in bucket 'n'.  Therefore,
                // empty entries MUST be sparse
                assert(it->second.mode() != modes::NULLED);

                // Whatever sparse bucket was tracked is not material since linear
                // probing rules dictate 'pos' doesn't have to sit in the ideal bucket area
                // providing we reached here after a series of other active members

                // 29JUN26 MB FIX: Still have lingering issue (#2 below) about mismatched bucket.  Despite
                // bucket appearing to be on the way out, that's not fully corroborated just yet
                traits::swap(*it, *pos);
                return it;
            }

            if((it = bump(it)) == begin_n)    return pos;
        }

        return pos;
    }

    control_pointer gc_active_ll_old(control_pointer pos, control_pointer begin, const size_type n)
    {
        assert(index(traits::key(*pos)) == n);        // Verify pos really is part of 'n'

        // FIX: Needs heavy revision for linear probing/wraparound realitites:
        // 1. cend() needs to switch to compare-against-start like we do with local_iterator
        // 2. swap dangerous because it can emit a mismatched second.bucket
        // 3. it < pos presumes that 'pos' is nearby correct bucket, but that is not a gauruntee with linear probing

        // look through other items in this bucket.  Not using local_iterator because he's
        // designed to skip over nulls, while we specifically are looking for those guys.
        // Also, we don't want to swap our active guy further down the bucket, only earlier
        for(control_pointer it = begin; it != container_.cend() && it < pos; ++it)
        {
            // if item is null (maybe) sparse
            if(is_empty(*it))
            {
                // if sparse, double check we're in the same bucket still
                if(it->second.marked_for_gc)
                {
                    // moving out of the bucket terminates the GC operation, no null slot found
                    // NOTE: we could make an extended mode which just reaches on forever, linear probing doesn't
                    // prohibit that at all - it's just crossing into a new bucket now we have to do some extra checks.
                    // a prune_sparse_ll may make that easier
                    if(it->second.bucket != n) return pos;
                }

                //it->swap(*pos);
                traits::swap(*it, *pos);
                return it;
            }
        }

        return pos;
    }

    /// perform garbage collection on the bucket containing this active pos, namely moving
    /// pos to first empty slot in bucket.
    /// @param pos entry to possibly move
    /// @returns potentially moved 'pos'
    control_pointer gc_active_ll(control_pointer pos)
    {
        const size_type n = index(traits::key(*pos));

        return gc_active_ll(pos, container_.begin() + n, n);
    }


#if UNIT_TESTING
public:
#endif
    //
    ///
    /// @brief prune_sparse_ll null out trailing tombstones, moving backward
    /// @param start DEBT: rename - this is end point
    /// @param pos where to begin nulling from
    /// @param n
    ///
    void prune_sparse_ll(control_pointer start, control_pointer pos)
    {
        start = rbump(start);

        for(; is_tombstone(*pos) && pos != start; pos = rbump(pos))
            tombstone_to_null(pos);
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
    template <class ...Args>
    explicit constexpr unordered_map(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

    ESTD_CPP_CONSTEXPR(14) void clear()
    {
        for(control_type& v : container_)
        {
            if(!is_empty(v))    destruct(&v);

            set_null(&v);       // Just in case he's tombstoned
        }
    }

    template <class K>
    mapped_type& operator[](K&& key)
    {
        find_result<control_pointer> found = find_ll(std::forward<K>(key));

        if(found.second != npos()) return found.first->second.mapped();

        return try_emplace(std::forward<K>(key)).first->second;
    }

    ESTD_CPP_CONSTEXPR(14) mapped_type& at(const key_type& key)
    {
        find_result<control_pointer> found = find_ll(key);

        // DEBT: Throw an exception if feature flag indicates to do so
        if(found.second == npos()) abort();

        return found.first->second.mapped();
    }

    ESTD_CPP_CONSTEXPR(14) const mapped_type& at(const key_type& key) const
    {
        find_result<const_control_pointer> found = find_ll(key);

        // DEBT: Throw an exception if feature flag indicates to do so
        if(found.second == npos()) abort();

        return found.first->second.mapped();
    }

    template <class K, class ...Args>
    pair<iterator, bool> emplace(K&& key, Args&&...args)
    {
        const size_type n = index(key);
        const insert_result ret = insert_precheck(std::forward<K>(key), n, false);

        if(ret.second)
        {
            new (ret.first) value_type(std::forward<K>(key), std::forward<Args>(args)...);

            find_and_mark_eol(bump(ret.first), n);
        }

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
        const key_type& key = value.first;
        const size_type n = index(key);
        const insert_result ret = insert_precheck(key, n, permit_duplicates);

        if(ret.second)
        {
            // We've made it here without reaching the end or bonking into another bucket,
            // we're good to go
            new (ret.first) value_type(value);

            find_and_mark_eol(bump(ret.first), n);
        }

        return wrap_result(ret);
    }

    template <class P>
    auto insert(P&& value, bool permit_duplicates = false) ->
        enable_if_t<is_constructible<value_type, P&&>::value, pair<iterator, bool>>
    {
        const key_type& key = value.first;
        const size_type n = index(key);
        const insert_result ret = insert_precheck(value.first, n, permit_duplicates);

        if(ret.second)
        {
            // We've made it here without reaching the end or bonking into another bucket,
            // we're good to go
            new (ret.first) value_type(std::forward<P>(value));

            find_and_mark_eol(bump(ret.first), n);
        }

        return wrap_result(ret);
    }

    template <class K, class M>
    pair<iterator, bool> insert_or_assign(const K& k, M&& obj)
    {
        find_result<control_pointer> found = find_ll(k);

        if(found.second != npos())
        {
            control_pointer cp = found.first;
            mapped_type& p = cp->second.mapped();
            new (&p) mapped_type(std::forward<M>(obj));
            return { { this, cast(cp) }, true };
        }

        // DEBT: Would prefer an emplace here, but it's not smart enough to reliably
        // sort out class K
        return insert({k, std::forward<M>(obj)});
    }

    iterator gc_active(iterator pos)
    {
        return { this, (pointer) gc_active_ll(cast_control(estd::addressof(*pos))) };
    }

    // Turns this sparse 'pos' to completely deleted 'null'
    static void tombstone_to_null(control_pointer control);

    // Turn this tombstone 'pos' to completely deleted 'null'
    static void tombstone_to_null(pointer pos)
    {
        tombstone_to_null(cast_control(pos));
    }

    /// For a given bucket, search until null or wraparound occurs and if a
    /// tombstone is found along the way, mark it with eol
    /// @param control - where to start search from
    /// @param n - bucket
    /// @return true when eol actually got marked
    bool find_and_mark_eol(control_pointer control, unsigned n);

    /// For a given bucket, search until null or wraparound occurs and move
    /// null forward if possible
    bool find_and_mark_null(control_pointer control, unsigned n);

    // Conforms to spec in that:
    // "References and iterators to the erased elements are invalidated.
    //  Other iterators and references are not invalidated. "
    void erase_ll(find_result<control_pointer> pos, bool auto_prune = true);

    // equivelant to erase with iterator, but merely takes direct value_type*
    void erase_ll(pointer pos)
    {
        erase_ll({ cast_control(pos), index(traits::key(*pos)) });
    }

    // https://en.cppreference.com/w/cpp/container/unordered_map/erase
    // NOTE: example implies internal ordering of unordered_map is predictable, which
    // on one hand feels reasonable, but on the other seems to conflict with the notion
    // that we are officially unordered.
    /// @param pos
    /// @return Iterator following the removed element
    iterator erase(iterator pos)
    {
        pointer p = estd::addressof(*pos);

        erase_ll(p);

        //return { this, skip_empty_old(p + 1) };
        return { this, skip_empty_new(bump(p)) };
    }

    void erase_and_gc(iterator pos)
    {
        base_type::erase_and_gc_ll(cast_control(estd::addressof(*pos)));
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
