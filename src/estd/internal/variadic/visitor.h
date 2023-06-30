#pragma once

#include "fwd.h"
//#include "../fwd/variant.h"
#include "selector.h"
//#include "../../optional.h"


#if __cpp_variadic_templates
namespace estd {

namespace internal {

struct legacy_visit_instance_functor
{
    template <size_t I, class T, class Fg, class Tg, class F, class ...TArgs>
    //constexpr
    bool operator()(variadic::visitor_index<I, T> vi,
        Fg&& getter,
        Tg&& source,
        F&& f,
        TArgs&&...args) const
    {
        auto& value = getter(vi, source);
        variadic::visitor_instance<I, T> vi2{value};

        return f(
            //variadic::visitor_instance<I, T>{getter(vi, source)},
            vi2,
            std::forward<TArgs>(args)...);
    }
};

template <size_t I>
struct visitor_index : in_place_index_t<I>
{
    static constexpr size_t index = I;
};

// DEBT: This name and namespace location are confusing in respect to
// the existence of value_visitor
template <size_t I, class T, T v>
struct visitor_value :
    visitor_index<I>,
    integral_constant<T, v>
{

};


}

namespace variadic {

template <size_t I, class T>
struct visitor_index :
    internal::visitor_index<I>,
    in_place_type_t<T>,
    type_identity<T>
{
};

template <size_t I, class T>
struct visitor_instance : visitor_index<I, T>
{
    T& value;

    typedef T value_type;

    visitor_instance(const visitor_instance&) = default;
    //visitor_instance(visitor_instance&&) noexcept = default;

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


template <class T, T ...Values>
struct value_visitor
{
    template <size_t I>
    using get = internal::get_index_finder<I, T, Values...>;

    template <size_t I>
    using indexer = internal::visitor_value<I, T, get<I>::value>;

    template <size_t I,
        class enabled = enable_if_t<(I == sizeof...(Values))>,
        class... TArgs,
        class F>
    static constexpr short visit(F&&, TArgs&&...) { return -1; }

    template <size_t I = 0, class F,
        class enabled = enable_if_t<(I < sizeof...(Values))>,
        class... TArgs>
    constexpr static int visit(F&& f, TArgs&&...args)
    {
        return f(indexer<I>{}, std::forward<TArgs>(args)...) ?
            I :
            visit<I + 1>(std::forward<F>(f), std::forward<TArgs>(args)...);
    }
};

template <typename... Types>
struct type_visitor
{
    //typedef layer1::optional<size_t, internal::variant_npos()> return_type;

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

#if LEGACY
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
#endif

    template <class F, class Fg, class Tg, class... TArgs>
    static int visit_instance(F&& f, Fg&& getter, Tg&& t, TArgs&&...args)
    {
        return visit(internal::legacy_visit_instance_functor{},
            std::forward<Fg>(getter),
            std::forward<Tg>(t),
            std::forward<F>(f),
            std::forward<TArgs>(args)...);
    }

    template <class TEval>
    using select = detail::selector<sizeof...(Types), TEval, Types...>;
};


}

}
#endif
