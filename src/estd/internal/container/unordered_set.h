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
    using typename base_type::insert_result;
    using base_type::insert_precheck;
    using base_type::skip_null;
    using base_type::npos;
    using base_type::find_ll;
    using typename base_type::control_pointer;

    template <class Pointer>
    using find_result = typename base_type::template find_result<Pointer>;

    // Not ready yet, and maybe we can consolidate too
    static ESTD_CPP_CONSTEXPR(14) void set_null(control_pointer v)
    {
        nullable{}.set(&v->first);
    }

    static void destruct(control_pointer v)
    {
        v->~value_type();
        set_null(v);
    }

    void erase_ll(find_result<control_pointer> pos)
    {
        control_pointer control = pos.first;

        destruct(control);
    }

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
    pair<iterator, bool> insert(const_reference value)
    {
        const insert_result ret = insert_precheck(value, false);

        if(ret.second)
            new (ret.first) value_type(value);

        return { { ret.first }, ret.second };
    }

    iterator erase(iterator pos)
    {
        erase_ll({ cast_control(pos.value()), index(*pos) });

        return { this, skip_null(pos.value() + 1) };
    }

    // NOTE: Feels like this might be able to live in base class
    size_type erase(const key_type& key)
    {
        find_result<control_pointer> found = find_ll(key);

        if(found.second == npos()) return 0;

        erase_ll(found);
        return 1;
    }
};

}}
