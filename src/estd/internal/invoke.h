#pragma once

#ifdef FEATURE_CPP_VARIADIC

#include "invoke_result.h"

// TODO: Adjust so that earlier versions can use
#if __cplusplus >= 201703L

namespace estd {

namespace detail {
template <class T>
constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

template <class T, class Type, class T1, class... Args>
decltype(auto) INVOKE(Type T::* f, T1&& t1, Args&&... args)
{
    if constexpr (estd::is_member_function_pointer_v<decltype(f)>) {
        if constexpr (estd::is_base_of_v<T, std::decay_t<T1>>)
            return (std::forward<T1>(t1).*f)(std::forward<Args>(args)...);
        else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
            return (t1.get().*f)(std::forward<Args>(args)...);
        else
            return ((*std::forward<T1>(t1)).*f)(std::forward<Args>(args)...);
    } else {
        static_assert(estd::is_member_object_pointer_v<decltype(f)>);
        static_assert(sizeof...(args) == 0);
        if constexpr (estd::is_base_of_v<T, estd::decay_t<T1>>)
            return std::forward<T1>(t1).*f;
        else if constexpr (is_reference_wrapper_v<estd::decay_t<T1>>)
            return t1.get().*f;
        else
            return (*std::forward<T1>(t1)).*f;
    }
}

template <class F, class... Args>
decltype(auto) INVOKE(F&& f, Args&&... args)
{
      return std::forward<F>(f)(std::forward<Args>(args)...);
}

} // namespace detail

template< class F, class... Args>
estd::invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
  noexcept(std::is_nothrow_invocable_v<F, Args...>)
{
    return detail::INVOKE(std::forward<F>(f), std::forward<Args>(args)...);
}


}

#endif

#endif