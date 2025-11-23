#pragma once

#include "../../../new.h"

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace estd { namespace detail { namespace impl {

template <typename Result, typename... Args, fn_options o>
struct function_virtual<Result(Args...), o>
{
    static constexpr fn_options options = o;

    struct model_base
    {
        virtual Result operator()(Args&&...args) = 0;
        virtual ~model_base() = default;
#if FEATURE_ESTD_GH135
        virtual void copy_to(model_base*) = 0;
        virtual void move_to(model_base*) = 0;
        void destroy() { this->~model_base(); }
#endif
    };

    template <class F>
    struct model : model_base
    {
        constexpr explicit model(const F& u) :
            f(u)
        {}

        constexpr explicit model(F&& u) :
            f(std::forward<F>(u))
        {
        }

        F f;

        Result operator()(Args&&...args) override
        {
            return f(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        void copy_to(model_base* dest) override
        {
            new (dest) model(f);
        }

        void move_to(model_base* dest) override
        {
            new (dest) model(std::move(f));
        }
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

        Result operator()(Args&&...args) override
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        void copy_to(model_base* dest) override
        {

        }

        void move_to(model_base* dest) override
        {

        }
#endif
    };
};


}}}

#endif
