#pragma once

#include "fwd.h"
#include "concepts.h"
#include "selector.h"


#if __cpp_variadic_templates
namespace estd {

namespace internal {

struct legacy_visit_instance_functor
{
    template <size_t I, class T, class Fg, class Tg, class F, class ...TArgs>
    //constexpr
    bool operator()(variadic::v1::visitor_index<I, T> vi,
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

// DEBT: Use 'Concept' here to filter to 'tuple-like'
struct visit_tuple_functor
{
    template <size_t I, class T, class Tuple, class F, class ...TArgs>
    bool operator()(variadic::type<I, T>, Tuple& tuple, F&& f, TArgs&&...args) const
    {
        typename tuple_element<I, Tuple>::valref_type v = get<I>(tuple);

        return f(variadic::instance<I, T>{v}, std::forward<TArgs>(args)...);
    }

    template <size_t I, class T, class Tuple, class F, class ...TArgs>
    bool operator()(variadic::type<I, T>, const Tuple& tuple, F&& f, TArgs&&...args) const
    {
        typename tuple_element<I, Tuple>::const_valref_type v = get<I>(tuple);

        return f(variadic::instance<I, T>{v}, std::forward<TArgs>(args)...);
    }
};

template <size_t I>
struct visitor_index : in_place_index_t<I>
{
    static constexpr size_t index = I;
};

template <class T>
struct visitor_instance : in_place_type_t<T>
{
    ESTD_CPP_STD_VALUE_TYPE(T)

    reference value;

    visitor_instance(const visitor_instance&) = default;

    constexpr explicit visitor_instance(T& value) : value{value} {}
    
    pointer operator->() { return &value; }
    ESTD_CPP_CONSTEXPR_RET const_pointer operator->() const { return &value; }
};

}

namespace experimental {

template <class ...>
struct functor_mover;

template <>
struct functor_mover<>
{
    static constexpr size_t size = 0;

    template <class ...Args2>
    constexpr bool operator()(Args2&&...) const { return {}; }
};

template <class T, class ...Args>
struct functor_mover<T, Args...>
{
    using upward = functor_mover<Args...>;
    static constexpr size_t size = sizeof...(Args) + 1;

    // emits position in reverse
    template <class F, class ...Args2>
    void operator()(F&& f, Args2&&...args2)
    {
        f(variadic::type<size - 1, T>{}, std::forward<Args2>(args2)...);
        upward{}(std::forward<F>(f), std::forward<Args2>(args2)...);
    }
};

template <class ...Args>
struct forward_invoker
{
    static constexpr size_t size = sizeof...(Args);

    template <class F, int I, class T, class ...Args2>
    void operator()(variadic::type<I, T>, F&& f, Args2&&...args) const
    {
        f(variadic::type<size - (I + 1), T>{}, std::forward<Args2>(args)...);
    }

    template <class F>
    void invoke(F&& f)
    {
        functor_mover<Args...>{}(std::forward<F>(f));
    }
};



}

namespace variadic {

inline namespace v1 {
// DEBT: Not a great name, seeing as it interacts with 'value_visitor'
template <size_t I, class T, T v>
struct visitor_value :
    internal::visitor_index<I>,
    integral_constant<T, v>
{

};

template <size_t I, class T>
struct visitor_index :
    internal::visitor_index<I>,
    in_place_type_t<T>,
    type_identity<T>
{
};

template <size_t I, class T>
struct visitor_instance : visitor_index<I, T>,
    // NOTE: Not bothered that we have multiple inheritance for in_place_type_t since it's an empty
    // struct anyway
    internal::visitor_instance<T>
{
    visitor_instance(const visitor_instance&) = default;
    //visitor_instance(visitor_instance&&) noexcept = default;

    constexpr explicit visitor_instance(T& value) :
        internal::visitor_instance<T>(value) {}
};

// EXPERIMENTAL - not in other namespace because it's a specialization
template <class T>
struct visitor_instance<internal::variant_npos(), T> : internal::visitor_instance<T>
{
    visitor_instance(const visitor_instance&) = default;

    constexpr explicit visitor_instance(T& value) :
        internal::visitor_instance<T>(value) {}

    template <size_t I>
    constexpr visitor_instance(const visitor_instance<I, T>& vi) :
        visitor_instance(vi.value)
    {}
};


}

// DEBT: Move this out, if we can.  Specialized usage of visitor_index etc makes that hard though
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
    using value = v1::visitor_value<I, T, get<I>::value>;

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
        return f(value<I>{}, std::forward<TArgs>(args)...) ?
            I :
            visit<I + 1>(std::forward<F>(f), std::forward<TArgs>(args)...);
    }
};

template <typename... Types>
struct type_visitor
{
    //typedef layer1::optional<size_t, internal::variant_npos()> return_type;

    static constexpr const size_t size = sizeof...(Types);

    template <size_t I = 0,
            class enabled = enable_if_t<(I == size)>,
            class... TArgs,
#if __cpp_concepts
            concepts::ClassVisitorFunctor<TArgs...> F>
#else
            class F>
#endif
    static constexpr short visit(F&&, TArgs&&...) { return -1; }

    template <size_t I = 0, class F,
            class enabled = enable_if_t<(I < size)>,
            class... TArgs>
    static int visit(F&& f, TArgs&&...args)
    {
        if(f(visitor_index<I, internal::type_at_index<I, Types...>>{}, std::forward<TArgs>(args)...))
            return I;

        return visit<I + 1>(std::forward<F>(f), std::forward<TArgs>(args)...);
    }

    template <int I = size - 1,
        class enabled = enable_if_t<(I == -1)>,
        class... Args>
    constexpr static short visit_reverse(Args&&...) { return -1; }

    template <int I = size - 1, class F,
        class enabled = enable_if_t<(I >= 0)>,
        class... Args>
    static int visit_reverse(F&& f, Args&&...args)
    {
        if(f(type<I, internal::type_at_index<I, Types...>>{}, std::forward<Args>(args)...))
            return I;

        return visit_reverse<I - 1>(std::forward<F>(f), std::forward<Args>(args)...);
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

    template <class Eval>
    using select = detail::selector<size, Eval, Types...>;
};


}

}
#endif
