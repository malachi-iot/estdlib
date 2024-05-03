#pragma once

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace estd { namespace detail { namespace impl {

template <typename TResult, typename... TArgs>
struct function_virtual<TResult(TArgs...)>
{
    struct model_base
    {
        virtual TResult _exec(TArgs...args) = 0;
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

        TResult _exec(TArgs...args) override
        {
            return f(std::forward<TArgs>(args)...);
        }
    };
};


}}}

#endif