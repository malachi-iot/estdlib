#pragma once

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace estd { namespace detail { namespace impl {

template <typename Result, typename... Args>
struct function_virtual<Result(Args...)>
{
    struct model_base
    {
        virtual Result operator()(Args...args) = 0;
        virtual ~model_base() = default;
    };

    template <class F>
    struct model : model_base
    {
        constexpr explicit model(F&& u) :
            f(std::forward<F>(u))
        {
        }

        F f;

        Result operator()(Args...args) override
        {
            return f(std::forward<Args>(args)...);
        }
    };


    // 13MAY24 MB EXPERIMENTAL replacement for 'thisify'
    template <class T, Result (T::*f)(Args...)>
    struct method_model : model_base
    {
        constexpr explicit method_model(T* t) :
            object_{t}
        {}

        T* const object_;

        Result operator()(Args...args) override
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }
    };
};


}}}

#endif