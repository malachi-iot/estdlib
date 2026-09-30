#pragma once

#include "../fwd/variant.h"
#include "../raw/utility.h"
#include "../raw/variant.h"
#include "../feature/variant.h"

#include "../variadic.h"

#include "../../new.h"

#if __cpp_exceptions
#include <exception>
#endif

namespace estd {

#if __cpp_exceptions
class bad_variant_access : std::exception
{
public:
};
#endif

template <class... Types>
struct variant_size<internal::variant_storage<Types...> > : variadic::size<Types...> {};

template <size_t I, class... Types>
struct variant_alternative<I, internal::variant_storage<Types...> > :
    type_identity<internal::type_at_index<I, Types...>> { };

namespace internal {

#if FEATURE_ESTD_VARIANT_PERMISSIVE_ASSIGNMENT
template <class T, class U>
using is_variant_assignable = is_constructible<T, U>;
#else
template <class T, class U>
using is_variant_assignable = bool_constant<
    is_constructible<T, U>::value &
    is_assignable<T&, U>::value>;
#endif

// https://en.cppreference.com/cpp/language/new
// https://en.cppreference.com/cpp/memory/construct_at
// https://stackoverflow.com/questions/41580022/constexpr-placement-new

struct variant_storage_getter_functor
{
    template <size_t I, class T, class ...Types>
    ESTD_CPP_CONSTEXPR(14) T& operator()(
        variadic::visitor_index<I, T>, internal::variant_storage<Types...>& vs) const
    {
        return get<I>(vs);
    }

    template <size_t I, class T, class ...Types>
    ESTD_CPP_CONSTEXPR(14) T& operator()(variadic::visitor_index<I, T>, variant<Types...>& vs) const
    {
        // DEBT: A bit of a cheat
        return *get_ll<I>(vs);
    }
};

template <class T>
struct converting_constructor_functor
{
    T& t;

    template <class T_i>
    constexpr bool operator()(T_i*) const { return false; }

    template <class T_i,
        enable_if_t<estd::is_constructible<T_i, T>::value, bool> = true>
    bool operator()(T_i* t_i)
    {
        new (t_i) T_i(std::forward<T>(t));
        return true;
    }
};


struct converting_constructor_functor2
{
    template <size_t I, class TVariant, class T>
    constexpr bool operator()(in_place_index_t<I>, TVariant, T&&) const { return false; }

    template <size_t I, class T_i, class TVariant, class T,
        enable_if_t<estd::is_constructible<T_i, T>::value, bool> = true>
    bool operator()(variadic::visitor_index<I, T_i>, TVariant& v, T&& t)
    {
        new (v.template get<I>()) T_i(std::forward<T>(t));
        return true;
    }
};

// NOTE: Using regular functions for F&& style functors doesn't
// seem to work, perhaps in particular due to class T parameter?
struct destroyer_functor
{
    template <size_t I, class T>
    bool operator()(variadic::visitor_instance<I, T> vi)
    {
        vi.value.~T();

        return true;
    }
};

}   // namespace internal

}   // namespace estd
