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
    template <size_t I, class T, class Tuple, class F, class ...Args>
    bool operator()(variadic::type<I, T>, Tuple& tuple, F&& f, Args&&...args) const
    {
        typename tuple_element<I, Tuple>::valref_type v = get<I>(tuple);

        return f(variadic::instance<I, T>{v}, std::forward<Args>(args)...);
    }

    template <size_t I, class T, class Tuple, class F, class ...Args>
    bool operator()(variadic::type<I, T>, const Tuple& tuple, F&& f, Args&&...args) const
    {
        using type = typename tuple_element<I, Tuple>::const_valref_type;
        type v = get<I>(tuple);

        return f(variadic::instance<I, const T>{v}, std::forward<Args>(args)...);
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
    using value_type = estd::remove_reference_t<T>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    reference value;

    visitor_instance(const visitor_instance&) = default;

    constexpr explicit visitor_instance(T& value) : value{value} {}
    
    pointer operator->() { return &value; }
    ESTD_CPP_CONSTEXPR_RET const_pointer operator->() const { return &value; }
};

// string literals like to appear this way.  We need a specialization here
// because acquiring pointer to this guy is tricky
template <class T, unsigned N>
struct visitor_instance<T (&)[N]> : in_place_type_t<T (&)[N]>
{
    using reference = T (&)[N];

    reference value;

    constexpr explicit visitor_instance(reference value) : value{value} {}

    // NOTE: No arrow operator - pointer is hard to do and arrow wouldn't
    // make sense for this type anyway
};


}

namespace experimental {

template <class ...>
struct functor_mover;

template <class ...>
struct functor_mover2;

template <class T, class ...Types>
struct functor_mover2<variadic::types<T, Types...>, T>
{
    using types_ = variadic::types<T, Types...>;
    using upward = functor_mover2<variadic::types<Types...>, typename types_::first>;
};

template <>
struct functor_mover<>
{
    template <class ...Args>
    constexpr static bool invoke(Args&&...) { return false; }
};

template <class T, class ...Types>
struct functor_mover<T, Types...>
{
    using upward = functor_mover<Types...>;

    // emits position in reverse
    template <class F, class ...Args>
    constexpr static bool invoke(F&& f, Args&&...args)
    {
        return f(variadic::type<sizeof...(Types), T>{}, std::forward<Args>(args)...) ?
            true :
            upward::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
};

template <class ...Types>
struct forward_invoker
{
    static constexpr size_t size = sizeof...(Types);

    template <class F, size_t I, class T, class ...Args>
    bool operator()(variadic::type<I, T>, F&& f, int* selected, Args&&...args) const
    {
        constexpr size_t index = size - (I + 1);

        if(!f(variadic::type<index, T>{}, std::forward<Args>(args)...)) return false;

        *selected = index;
        return true;
    }

    template <class F, class ...Args>
    int invoke(F&& f, Args&&...args) const
    {
        int selected = -1;
        functor_mover<Types...>::invoke(*this,
            std::forward<F>(f),
            &selected,
            std::forward<Args>(args)...);
        return selected;
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

    template <size_t I = size,
        class enabled = enable_if_t<(I == 0)>,
        class... Args>
    constexpr static short visit_reverse(Args&&...) { return -1; }

    template <size_t I = size, class F,
        class enabled = enable_if_t<(I > 0)>,
        class... Args>
    static int visit_reverse(F&& f, Args&&...args)
    {
        if(f(type<I-1, internal::type_at_index<I-1, Types...>>{}, std::forward<Args>(args)...))
            return I - 1;

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
