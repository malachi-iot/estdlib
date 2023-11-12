#pragma once

#include "fwd.h"

namespace estd {

namespace internal {
template<unsigned index, typename First, typename... Rest>
struct GetImpl;

// 'GetImpl' lifted and adapted from https://gist.github.com/IvanVergiliev/9639530
// TODO: use new variadic support area instead of this

template<unsigned index, typename First, typename... Rest>
struct GetImpl : GetImpl<index - 1, Rest...>
{
    /*
    typedef GetImpl<index - 1, Rest...> base_type;

    static typename base_type::const_valref_type value(const tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }

    static typename base_type::valref_type value(tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }   */
};

// cascades down eventually to this particular specialization
// to retrieve the 'first' item
template<typename First, typename... Rest>
struct GetImpl<0, First, Rest...>
{
    //typedef First first_type;
    //using tuple_type = tuple<First, Rest...>;
    using valref_type = typename tuple<First>::valref_type;
    using const_valref_type = typename tuple<First>::const_valref_type;

    static const_valref_type value(const tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static valref_type value(tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static First&& value(tuple<First, Rest...>&& t)
    {
        return t.first();
    }
};


}

template<int index, typename... Types>
typename tuple_element<index, tuple<Types...> >::const_valref_type get(const tuple<Types...>& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

template<int index, typename... Types>
typename tuple_element<index, tuple<Types...> >::valref_type get(tuple<Types...>& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

template<int index, typename... Types>
tuple_element_t<index, tuple<Types...> >&& get(tuple<Types...>&& t)
{
    return internal::GetImpl<index, Types...>::value(std::move(t));
}

template<int index, typename... Types>
const tuple_element_t<index, tuple<Types...> >&& get(const tuple<Types...>&& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

}
