#pragma once

#include "../type_traits.h"
#include "../utility.h"

namespace estd {

template <class U>
class reference_wrapper;

namespace detail {
template <class T>
struct is_reference_wrapper : estd::false_type {};
template <class U>
struct is_reference_wrapper<estd::reference_wrapper<U> > : estd::true_type {};

template<class T>
struct invoke_impl {
    template<class F, class... Args>
    static auto call(F&& f, Args&&... args)
        -> decltype(std::forward<F>(f)(std::forward<Args>(args)...));
};

template<class B, class MT>
struct invoke_impl<MT B::*> {
    template<class T, class Td = typename estd::decay<T>::type,
        class = typename estd::enable_if<estd::is_base_of<B, Td>::value>::type
    >
    static auto get(T&& t) -> T&&;

    template<class T, class Td = typename estd::decay<T>::type,
        class = typename estd::enable_if<is_reference_wrapper<Td>::value>::type
    >
    static auto get(T&& t) -> decltype(t.get());

    template<class T, class Td = typename estd::decay<T>::type,
        class = typename estd::enable_if<!estd::is_base_of<B, Td>::value>::type,
        class = typename estd::enable_if<!is_reference_wrapper<Td>::value>::type
    >
    static auto get(T&& t) -> decltype(*std::forward<T>(t));

    template<class T, class... Args, class MT1,
        class = typename estd::enable_if<estd::is_function<MT1>::value>::type
    >
    static auto call(MT1 B::*pmf, T&& t, Args&&... args)
        -> decltype((invoke_impl::get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...));

    template<class T>
    static auto call(MT B::*pmd, T&& t)
        -> decltype(invoke_impl::get(std::forward<T>(t)).*pmd);
};

template<class F, class... Args, class Fd = typename estd::decay<F>::type>
auto INVOKE(F&& f, Args&&... args)
    -> decltype(invoke_impl<Fd>::call(std::forward<F>(f), std::forward<Args>(args)...));

} // namespace detail

namespace detail {
template <typename AlwaysVoid, typename, typename...>
struct invoke_result { };
template <typename F, typename...Args>
struct invoke_result<decltype(void(detail::INVOKE(std::declval<F>(), std::declval<Args>()...))),
                 F, Args...> {
    using type = decltype(detail::INVOKE(std::declval<F>(), std::declval<Args>()...));
};
} // namespace detail


template <class F, class... ArgTypes>
struct invoke_result : detail::invoke_result<void, F, ArgTypes...> {};

template< class F, class... ArgTypes>
using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;

}


