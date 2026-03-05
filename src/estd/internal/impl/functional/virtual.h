#pragma once

#include "../../../new.h"
#include "../../rtto.h"

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace estd { namespace detail { namespace impl {

template <typename Result, typename... Args, fn_options o>
struct function_virtual<Result(Args...), o> : internal::rtto_base
{
    static constexpr fn_options options = o;

    struct model_base : rtto_base::virtual_base
    {
#if FEATURE_ESTD_FUNCTION_RVALUE
        virtual Result operator()(Args&&...args) = 0;
#else
        virtual Result operator()(Args...args) = 0;
#endif
        //virtual ~model_base() = default;
#if FEATURE_ESTD_GH135
#endif
    };

    template <class F>
    struct model : model_base
    {
        using rtto = internal::rtto<model>;

        constexpr explicit model(const F& u) :
            f(u)
        {}

        constexpr explicit model(F&& u) :
            f(std::forward<F>(u))
        {
        }

        ESTD_CPP_DEFAULT_RULE_OF_5(model)

        F f;

#if FEATURE_ESTD_FUNCTION_RVALUE
        Result operator()(Args&&...args) override
#else
        Result operator()(Args...args) override
#endif
        {
            return f(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        int copy_to(void* dest, int sz) const override
        {
            return rtto::copy(this, dest, sz);
        }

        int move_to(void* dest, int sz) override
        {
            return rtto::move(this, dest, sz);
        }

#if FEATURE_ESTD_RTTO_GET_METADATA
        const metadata* get_metadata() const override
        {
            return model_base::template get<model>();
        }
#endif
#endif
    };


    // 13MAY24 MB EXPERIMENTAL replacement for 'thisify'
    template <class T, Result (T::*f)(Args...)>
    struct method_model : model_base
    {
        constexpr explicit method_model(T* t) :
            object_{t}
        {}

        T* const object_;

#if FEATURE_ESTD_FUNCTION_RVALUE
        Result operator()(Args&&...args) override
#else
        Result operator()(Args...args) override
#endif
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        int copy_to(void* dest, int sz) const override
        {
            return ENOSYS;
        }

        int move_to(void* dest, int sz) override
        {
            return ENOSYS;
        }

#if FEATURE_ESTD_RTTO_GET_METADATA
        const metadata* get_metadata() const override
        {
            // 50% implemented
            return model_base::template get<method_model>();
        }
#endif
#endif
    };
};


}}}

#endif
