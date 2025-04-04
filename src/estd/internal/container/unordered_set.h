#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"

#include "unordered/base.h"

namespace estd {

namespace internal {

// NOTE: Was gonna use quadratic probing here, but 'Hash' signature might be incompatible
// TODO: If possible consolidate/dogfood in unordered_map

template <class Container,
    class Key = typename Container::value_type,
    class Hash = hash<Key>,
    Key Null = Key(),
    class KeyEqual = equal_to<Key> >
class unordered_set;

template <class Container, class Key, class Hash, Key Null, class KeyEqual>
class unordered_set : public unordered_base<Container, unordered_traits<Key, Key, Hash, KeyEqual>>
{
    using base_type = unordered_base<Container, unordered_traits<Key, Key, Hash, KeyEqual>>;
    using base_type::container_;

public:
    ESTD_CPP_STD_VALUE_TYPE(Key)

    using key_type = value_type;
    using hasher = typename base_type::hasher;
    using size_type = typename base_type::size_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

private:
    //size_type size_ = 0;

public:
    bool empty() const
    {
        const_iterator i = container_.begin();

        for(;i != container_.end(); ++i)
        {
            if(*i != Null) return false;
        }

        return true;
    }

    size_type size() const
    {
        //return set_.size();
        size_type sz = 0;

        for(const_reference v : container_)
            if(v != Null)  ++sz;

        return sz;
    }

    pair<iterator, bool> insert(const_reference value)
    {
        unsigned hashed = hasher{}(value) % container_.size();

        // linear probing
        iterator it = &container_[hashed];
        while(*it != Null)
        {
            if(++it == container_.end())
            {
                return { it, false };
            }
        }

        *it = value;

        return { it, true };
    }
};

}}
