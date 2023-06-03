#pragma once

#include "fwd.h"
#include "selector.h"
#include "../is_base_of.h"


#if __cpp_variadic_templates
namespace estd {

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
    using select = detail::selector<sizeof...(Types), TEval, Types...>;
};


}

}
#endif
