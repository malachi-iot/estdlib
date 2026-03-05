#pragma once

#include "../fwd/variant.h"
#include "../variadic/type_sequence.h"

#include "accessors.h"

#if __cplusplus >= 201703L
namespace estd { namespace internal {

// NOTE: Could have done "constexpr if" for different R types.  Specialization felt more
// natural, and we're prepped for c++11 compatibility (lean on estd::variadic in that case)

template <class R>
struct index_visitor
{
    // DEBT: Upgrade variadic visit to do fold expressions (when c++17)
    template <class Visitor, class Index, size_t ...Is>
    static constexpr R visit(Visitor&& visitor, Index index, index_sequence<Is...>, bool* found)
    {
        using return_type = R;
        return_type result;
        *found = ((Is == index ? (result = visitor(in_place_index<Is>), true) : false) || ...);
        return result;
    }
};


template <>
struct index_visitor<void>
{
    // DEBT: Upgrade variadic visit to do fold expressions (when c++17)
    template <class Visitor, class Index, size_t ...Is>
    static constexpr void visit(Visitor&& visitor, Index index, index_sequence<Is...>, bool* found)
    {
        *found = ((Is == index ? (visitor(in_place_index<Is>), true) : false) || ...);
    }
};

template <class R>
struct variant_visit
{
    template <class Visitor, class Variant, size_t ...Is>
    static ESTD_CPP_CONSTEXPR(17) R visit(Visitor&& visitor, Variant&& vv, index_sequence<Is...>)
    {
        bool found{};
        return index_visitor<R>::visit([&](auto vi)
            {
                return visitor(*get_ll<vi.index>(std::forward<Variant>(vv)));
            },
            vv.index(), index_sequence<Is...>{}, &found);
    }
};

template <class Visitor, class ...Types, class R>
constexpr R visit(Visitor&& visitor, variant<Types...>& vv)
{
    return variant_visit<R>::visit(
        std::forward<Visitor>(visitor),
        vv,
        index_sequence_for<Types...>{});
}

template <class Visitor, class ...Types, class R>
constexpr R visit(Visitor&& visitor, const variant<Types...>& vv)
{
    return variant_visit<R>::visit(
        std::forward<Visitor>(visitor),
        vv,
        index_sequence_for<Types...>{});
}

}

template <class Visitor>
constexpr void visit(Visitor&&) { }

template <class Visitor, class Variant, class ...Variants>
constexpr auto visit(Visitor&& visitor, Variant&& variant, Variants&&... variants)
{
    // DEBT: Unclear how to aggregate return types, so not even trying.  Also, I bet
    // order is incorrect here.  This means UB for multiple variants who have a return
    // type
    visit(std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);

    return internal::visit(
        std::forward<Visitor>(visitor),
        std::forward<Variant>(variant));
}


}

#endif
