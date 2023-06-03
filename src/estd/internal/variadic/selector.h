#pragma once

#include "fwd.h"
#include "type_sequence.h"
#include "integer_sequence.h"
#include "../is_base_of.h"

#if __cpp_variadic_templates
namespace estd {

namespace internal {

struct projected_result_tag {};

}

namespace variadic {

template <class T, bool v = true>
struct projected_result :
        type_identity<T>,
        internal::projected_result_tag
{
    constexpr static bool value = v;
};

namespace detail {

template <size_t size, class TEval>
struct selector<size, TEval>
{
    using indices = index_sequence<>;
    using types = variadic::types<>;
    using projected = variadic::types<>;

    using selected = variadic::types<>;
};


template <size_t size, class TEval, class T, class ...Types>
struct selector<size, TEval, T, Types...>
{
private:
    typedef selector<size, TEval, Types...> upward;

    static constexpr size_t index = ((size - 1) - sizeof...(Types));

    using evaluated = typename TEval::template evaluator<T, index>;
    static constexpr bool eval = evaluated::value;

    using projected_type = conditional_t<
        is_base_of<internal::projected_result_tag, evaluated>::value,
        typename evaluated::type, T>;

    using visitor_index = variadic::visitor_index<index, projected_type>;

    template <class TSequence, class T2>
    using prepend = internal::prepend_if<eval, TSequence, T2>;

public:
    using indices = conditional_t<eval,
        typename upward::indices::template prepend<index>,
        typename upward::indices>;

    using types = prepend<typename upward::types, T>;
    using projected = prepend<typename upward::projected, projected_type>;
    using selected = prepend<typename upward::selected, visitor_index>;

    static constexpr bool all = selected::size() == sizeof...(Types) + 1;
};


}

}

}
#endif
