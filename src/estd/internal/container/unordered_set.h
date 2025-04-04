#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../fwd/utility.h"

#include "unordered/base.h"

namespace estd {

namespace internal {

// NOTE: Was gonna use quadratic probing here, but 'Hash' signature might be incompatible
// TODO: If possible consolidate/dogfood in unordered_map

template <class Container, class Traits>
class unordered_set_base : public unordered_base<Container, Traits>
{
    using base_type = unordered_base<Container, Traits>;
    using typename base_type::nullable;
    using base_type::container_;

public:
    using typename base_type::key_type;

    ESTD_CPP_STD_VALUE_TYPE(key_type)

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
            if(nullable{}.is_null(*i) == false) return false;
        }

        return true;
    }

    size_type size() const
    {
        //return set_.size();
        size_type sz = 0;

        for(const_reference v : container_)
            if(nullable{}.is_null(v) == false)  ++sz;

        return sz;
    }

    pair<iterator, bool> insert(const_reference value)
    {
        unsigned hashed = hasher{}(value) % container_.size();

        // linear probing
        iterator it = &container_[hashed];
        while(nullable{}.is_null(*it) == false)
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
