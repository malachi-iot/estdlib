#pragma once

#include "../fwd/functional.h"
#include "../../cstddef.h"

namespace estd {

// Adapted from
// https://stackoverflow.com/questions/9065081/how-do-i-get-the-argument-types-of-a-function-pointer-in-a-variadic-template-cla
// Additional guidance from
// https://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda/7943765#7943765
template<typename T>
struct function_traits :
    // I confess I don't fully understand this part.  Document it when I do
    function_traits<decltype(&T::operator())>
{
};

template<typename R, typename ...Args>
struct function_traits<R(Args...)>
{
    static constexpr size_t nargs = sizeof...(Args);

    static constexpr bool is_method = false;
    static constexpr bool is_function = true;

    using function_type = R(Args...);

    using return_type = R;

    using tuple = estd::tuple<Args...>;

    template <size_t i>
    using arg = typename estd::tuple_element<i, tuple>::type;
};

template<typename R, typename ...Args>
struct function_traits<estd::detail::function<R(Args...)> > :
    function_traits<R(Args...)>
{
};

// Friendly to lambda-functors
template<class C, typename R, typename ...Args>
struct function_traits<R(C::*)(Args...) const> :
    function_traits<R(Args...)>
{
    using function_type = R(C::*)(Args...) const;
    using class_type = C;
    static constexpr bool is_method = true;
};


// Regular methods are happy with this one
template<class C, typename R, typename ...Args>
struct function_traits<R(C::*)(Args...)> :
    function_traits<R(Args...)>
{
    using function_type = R(C::*)(Args...);
    using class_type = C;
    static constexpr bool is_method = true;
};


// This one works well for functors from regular functions
template<typename R, typename ...Args>
struct function_traits<R (&)(Args...)> :
    function_traits<R(Args...)>
{

};

}