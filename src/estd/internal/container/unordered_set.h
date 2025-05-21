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
class unordered_set : public unordered_base<Container, Traits>
{
    using base_type = unordered_base<Container, Traits>;
    using typename base_type::nullable;
    using typename base_type::traits;
    using base_type::container_;
    using typename base_type::insert_result;
    using base_type::insert_precheck;
    using base_type::npos;
    using base_type::index;
    using base_type::find_ll;
    using typename base_type::control_pointer;

    template <class Pointer>
    using find_result = typename base_type::template find_result<Pointer>;

public:
    using typename base_type::key_type;

    ESTD_CPP_STD_VALUE_TYPE(key_type)

    using typename base_type::hasher;
    using typename base_type::size_type;
    using typename base_type::iterator;
    using typename base_type::const_iterator;

    pair<iterator, bool> insert(const_reference value)
    {
        const insert_result ret = insert_precheck(value, false);

        if(ret.second)
            new (ret.first) value_type(value);

        return { { this, ret.first }, ret.second };
    }

    size_type erase(const key_type& key)
    {
        find_result<control_pointer> found = find_ll(key);

        if(found.second == npos()) return 0;

        base_type::erase_and_gc_ll(found.first);
        return 1;
    }
};

}}
