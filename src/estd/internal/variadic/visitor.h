#pragma once

#include "fwd.h"
#include "type_sequence.h"
#include "../is_base_of.h"


#if __cpp_variadic_templates
namespace estd {

namespace internal {

struct eval_result_tag {};

template <bool v, class T>
struct projected_result :
        type_identity<T>,
        eval_result_tag
{
    constexpr static bool value = v;
};


template <unsigned size, class TEval>
struct visitor_helper_struct2<size, TEval>
{
    static constexpr ptrdiff_t selected = -1;

    // DEBT: Monostate may collide with seeked-for types
    typedef monostate selected_type;

    using selected_indices = index_sequence<>;
    using selected_types = type_sequence<>;
    using projected_types = type_sequence<>;

    // EXPERIMENTAL
    using selected2 = type_sequence<>;
};


template <unsigned size, class TEval, class T, class ...Types>
struct visitor_helper_struct2<size, TEval, T, Types...>
{
    typedef visitor_helper_struct2<size, TEval, Types...> upward;

    static constexpr size_t index = ((size - 1) - sizeof...(Types));

    using evaluated = typename TEval::template evaluator<T, index>;
    static constexpr bool eval = evaluated::value;
    static constexpr ptrdiff_t selected = eval ? index : upward::selected;

    using projected_type = conditional_t<
            is_base_of<eval_result_tag, evaluated>::value,
            typename evaluated::type, T>;
    using selected_type = conditional_t<eval, projected_type, typename upward::selected_type>;

    using selected_indices = conditional_t<eval,
            typename upward::selected_indices::template prepend<index>,
            typename upward::selected_indices>;

    using selected_types = conditional_t<eval,
            typename upward::selected_types::template prepend<T>,
            typename upward::selected_types>;

    using projected_types = conditional_t<eval,
            typename upward::projected_types::template prepend<projected_type>,
            typename upward::projected_types>;

    // EXPERIMENTAL
    using selected2 = conditional_t<eval,
            typename upward::selected2::template prepend<variadic::visitor_index<index, projected_type> >,
            typename upward::selected2>;
};


}

namespace variadic {

template <size_t I, class T>
struct visitor_index  :
        in_place_index_t<I>,
        in_place_type_t<T>,
        type_identity<T>
{
    static constexpr size_t index = I;
};

template <size_t I, class T>
struct visitor_instance : visitor_index<I, T>
{
    T& value;

    typedef T value_type;

    visitor_instance(const visitor_instance&) = default;
    constexpr explicit visitor_instance(T& value) : value{value} {}
};

#if __cpp_concepts
#include <concepts>

namespace concepts {

template <class T, class TProvider>
concept GetterFunctor = requires(T f, TProvider& provider)
{
    f(visitor_index<0, int>{}, provider);
};

template <class T, class ...TArgs>
concept ClassVisitorFunctor = requires(T f, TArgs&&...args)
{
    { f(visitor_index<0, int>{}, std::forward<TArgs>(args)...) } -> std::same_as<bool>;
};


template <class T, class ...TArgs>
concept InstanceVisitorFunctor = requires(T f, TArgs&&...args, int v)
{
    { f(visitor_instance<0, int>(v), std::forward<TArgs>(args)...) } -> std::same_as<bool>;
};

}
#endif


template <typename... Types>
struct visitor
{
    template <size_t I,
            class enabled = enable_if_t<(I == sizeof...(Types))>,
            class... TArgs,
#if __cpp_concepts
            concepts::ClassVisitorFunctor<TArgs...> F>
#else
            class F>
#endif
    static constexpr short visit(F&&, TArgs&&...) { return -1; }

    template <size_t I = 0, class F,
            class enabled = enable_if_t<(I < sizeof...(Types))>,
            class... TArgs>
    static int visit(F&& f, TArgs&&...args)
    {
        if(f(visitor_index<I, internal::type_at_index<I, Types...>>{}, std::forward<TArgs>(args)...))
        return I;

        return visit<I + 1>(std::forward<F>(f), std::forward<TArgs>(args)...);
    }


#if __cpp_concepts
    template <size_t I, class... TArgs,
            concepts::InstanceVisitorFunctor<TArgs...> F, class T,
            concepts::GetterFunctor<T&> Fg,
#else
            template <size_t I, class F, class Fg, class T, class... TArgs,
#endif
            class enabled = enable_if_t<(I == sizeof...(Types))> >
    static constexpr short visit_instance(F&&, Fg&&, T&, TArgs&&...) { return -1; }

    template <size_t I = 0, class F, class Fg, class T,
            class enabled = enable_if_t<(I < sizeof...(Types))>,
            class... TArgs>
    static int visit_instance(F&& f, Fg&& getter, T& t, TArgs&&...args)
    {
        typedef internal::type_at_index<I, Types...> value_type;
        visitor_instance<I, value_type> vi{getter(visitor_index<I, value_type>{}, t)};

        if(f(vi, std::forward<TArgs>(args)...))
            return I;

        return visit_instance<I + 1>(
                std::forward<F>(f),
                std::forward<Fg>(getter),
                t,
                std::forward<TArgs>(args)...);
    }

    template <class TEval>
    using select = variadic::selector<TEval, Types...>;
};


template <class TEval, class ...Types>
struct selector
{
    typedef internal::visitor_helper_struct2<sizeof...(Types), TEval, Types...> vh_type;

    static constexpr ptrdiff_t selected = vh_type::selected;
    static constexpr ptrdiff_t index = selected;

    using selected_type = typename vh_type::selected_type;
    using selected_types = typename vh_type::selected_types;
    using projected_types = typename vh_type::projected_types;
    using selected_indices = typename vh_type::selected_indices;

    // EXPERIMENTAL
    using selected2 = typename vh_type::selected2;

    static constexpr unsigned found = selected_indices::size();
    static constexpr bool multiple = found > 1;
    static constexpr bool all = found == sizeof...(Types);

    // DEBT: fix signed/unsigned here
    using visitor_index = variadic::visitor_index<(unsigned)selected, selected_type>;
};

}

}
#endif
