#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"
#include "../array.h"

namespace estd {

namespace internal {

// DEBT: Hard-wired to layer1 stlye

// DEBT: This guy wants to play with estd::layer1::optional you can feel it
// We are doing this rather than a Null = T{} from the get go because some T won't
// play nice in that context
template <class T>
struct nullable_traits
{
    static constexpr bool is_null(const T& value)
    {
        return value == T{};
    }

    // DEBT: Don't want to use this, just helpful for init and erase
    static constexpr T get_null() { return T(); }

    static void set_null(T* value)
    {
        *value = T{};
    }
};

template <
    unsigned sz,
    class Key,
    class T,
    class Hash = hash<Key>,
    class Nullable = nullable_traits<Key>,
    class KeyEqual = equal_to<Key> >
class unordered_map;

template <unsigned N, class Key, class T, class Hash, class Nullable, class KeyEqual>
class unordered_map :
    protected Hash,  // EBO
    protected KeyEqual  // EBO
{
public:
    // The more collisions and/or duplicates you expect, the bigger this wants to be.
    // Idea being if you have two buckets near each other of only size 1, you'll never
    // have room to insert a collision/duplicate
    static constexpr unsigned bucket_depth = 4;

    // DEBT: Do static assert to make sure N is evenly divisible by bucket_depth

    using key_type = Key;
    using mapped_type = T;
    using value_type = estd::pair<const Key, T>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using hasher = Hash;
    using size_type = unsigned;

    struct end_local_iterator
    {
        const_iterator it_;
    };

    // DEBT: This sucks, a glorified const_cast
    using cheater_iterator = estd::pair<Key, T>*;
    using end_iterator = monostate;

    //using local_iterator = iterator;
    //using const_local_iterator = const_iterator;

    static constexpr size_type max_bucket_count()
    {
        return N / bucket_depth;
    }

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

    ///
    /// @param v
    /// @remark Does not run destructor
    static void set_null(reference v)
    {
        // DEBT: I hate const_cast'ing
        Nullable{}.set_null(const_cast<key_type*>(&v.first));
    }

    // FIX: We need this semi-initialized, with keys all being Null
    uninitialized_array<value_type, N> container_;

    // DEBT: Doesn't handle non-empty hasher
    template <class K>
    static constexpr size_type index(const K& key)
    {
        return bucket_depth * hasher{}(key) % max_size();
    }

    // indicates whether already-hashed (lhs) matches to-hash (rhs)
    static constexpr bool match(size_type lhs, const key_type& rhs)
    {
        return lhs == index(rhs);
    }

    template <class LocalIt>
    struct local_iterator_base
    {
        using this_type = local_iterator_base;

        // bucket designator
        const size_type n_;

        LocalIt it_;

        constexpr value_type operator*() const { return *it_; }

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
            return *this;
        }

        this_type operator++(int)
        {
            ++it_;

            return { n_, it_ - 1 };
        }
    };

    template <class K>
    pair<iterator, bool> insert_precheck(const K& key, bool permit_duplicates)
    {
        const size_type idx = index(key);

        // linear probing

        iterator it = &container_[idx];
        // While we don't have null, keep moving forward over occupied
        // spots
        for(;is_null(*it) == false; ++it)
        {
            // if we get to the complete end, that's a fail
            // if we've moved to the next bucket, that's also a fail
            if(it == cend() || index(it->first) != idx)
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
    using local_iterator = local_iterator_base<iterator>;
    using const_local_iterator = local_iterator_base<const_iterator>;

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
    constexpr const_iterator cend() const { return container_.end(); }
    constexpr const_iterator cbegin() const { return container_.begin(); }

    void clear()
    {
        for(reference v : container_)
        {
            v.second.~mapped_type();
            set_null(v);
        }
    }

    static constexpr size_type max_size() { return N; }

    // DEBT: May be a deviation since our buckets are a little more fluid, but I think
    // it conforms to spec
    template <class K>
    constexpr size_type bucket(const K& key) const
    {
        return index(key);
    }

    mapped_type& operator[](const key_type& key)
    {
        iterator found = find(key);

        if(found != cend()) return found->second;

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
    template <class K>
    pair<iterator, bool> try_emplace(const K& key)
    {
        pair<iterator, bool> ret = insert_precheck(key, false);

        // pair requires two parameters to construct, PLUS it's a const key.
        // fortunately, since we presume it's a trivial-ish type (no dtor)
        // we can brute force the key assignment
        if(ret.second)
            reinterpret_cast<cheater_iterator>(ret.first)->first = key;

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

        if(found != cend())
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
        return { n, &container_[n] };
    }

    const_local_iterator cbegin(size_type n) const
    {
        return { n, &container_[n] };
    }

    constexpr end_local_iterator end(size_type) const
    {
        return { cend() };
    }

    constexpr end_local_iterator cend(size_type) const
    {
        return { cend() };
    }

    // NOTE: This works, but you'd prefer to avoid it and iterate yourself directly
    size_type bucket_size(size_type n) const
    {
        unsigned counter = 0;

        for(const_local_iterator it = cbegin(n); it != end(n); ++it)
            ++counter;

        return counter;
    }

    template <class K>
    constexpr bool contains(const K& key) const
    {
        return find(key) != cend();
    }

    // deviates from std in that other iterators part of this bucket could be invalidated
    // DEBT: We do want to return 'iterator', it's just unclear from spec how that really works
    void erase(iterator pos)
    {
        iterator start = pos;

        set_null(*pos);
        pos->second.~mapped_type();

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

    size_type erase(const key_type& key)
    {
        iterator found = find(key);

        if(found == cend()) return 0;

        erase(found);
        return 1;
    }

    template <class K>
    size_type count(const K& x) const
    {
        size_type n = index(x);
        unsigned counter = 0;

        for(const_local_iterator it = cbegin(n); it != end(n); ++it)
        {
            if(KeyEqual{}(x, it->first))
                ++counter;
        }

        return counter;
    }

    template <class K>
    const_iterator find(const K& x) const
    {
        size_type n = index(x);

        for(const_local_iterator it = cbegin(n); it != end(n); ++it)
            if(KeyEqual{}(x, it->first))
                return it;

        return cend();
    }

    template <class K>
    iterator find(const K& x)
    {
        size_type n = index(x);

        for(local_iterator it = begin(n); it != end(n); ++it)
            if(KeyEqual{}(x, it->first))
                return it;

        return end();
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
