#pragma once

#include "../../type_traits.h"
#include "../../rtto.h"

#include "fwd.h"

// Arduino compatibility
#pragma push_macro("F")
#undef F

namespace estd { namespace detail { namespace impl {

template <typename Result, typename... Args, fn_options o>
struct function_fnptr2<Result(Args...), o> : public internal::rtto_base
{
    static constexpr fn_options options = o;
    // DEBT: Make this optional and default to off
    static constexpr bool has_utility = true; //o & fn_options::FN_COPY;

    struct no_utility_base
    {
        ESTD_CPP_DEFAULT_RULE_OF_5(no_utility_base)
        constexpr explicit no_utility_base(utility_type) {}
    };

    using model_rtto_base = conditional_t<has_utility,
        base,
        no_utility_base>;

    // this is a slightly less fancy more brute force approach to try to diagnose esp32
    // woes
    struct model_base : model_rtto_base
    {
        using base_type = model_rtto_base;

#if FEATURE_ESTD_FUNCTION_RVALUE
        using function_type = Result (*)(void*, Args&&...);
#else
        using function_type = Result (*)(void*, Args...);
#endif

        const function_type f_;

        constexpr explicit model_base(
            function_type f,
            utility_type u) :
            base_type(u),
            f_(f)
        {}

        model_base(const model_base& copy_from) = default;

        constexpr model_base(model_base&& move_from) noexcept :
            base_type(std::move(move_from)),
            f_(std::move(move_from.f_))
        {}

        inline Result _exec(Args&&...args)
        {
            return f_(this, std::forward<Args>(args)...);
        }

        inline Result operator()(Args&&...args)
        {
            return f_(this, std::forward<Args>(args)...);
        }
    };

    template <typename F>
    struct model : model_base
    {
        using base_type = model_base;
        using typename base_type::function_type;
        using rtto = internal::rtto<model>;

#if FEATURE_ESTD_RTTO_GET_INVOKE
        // EXPERIMENTAL
        static int utility(modes mode, void* p0, int p1, void* p2)
        {
            if(mode == GET_INVOKE)
            {
                // At this point, you have to wonder if a vtable is better
                *static_cast<function_type*>(p0) = &model::exec;
                return 0;
            }

            return rtto::utility(mode, p0, p1, p2);
        }
#else
        using utility = rtto::utility;
#endif

        //template <typename U>
#if !__GNUC__ || __clang__ || __GNUC__ > 8
        ESTD_CPP_CONSTEXPR(14)
#endif
        explicit model(F&& u) :
            base_type(
                static_cast<function_type>(&model::exec),
                utility),
            f(std::forward<F>(u))
        {
        }

#if __clang__ || __GNUC__ > 8
#endif
        explicit model(const F& u) :
            base_type(
                static_cast<function_type>(&model::exec),
                utility),
            f(u)
        {
        }

        ESTD_CPP_DEFAULT_RULE_OF_5(model);

        F f;

        /*
        static void dtor(void* _this)
        {
            ((model*)_this)->f.~F();
        }   */

        // TODO: Consolidate different models down to a model_base since they
        // all need this exec function
        ESTD_CPP_CONSTEXPR(14) Result operator()(Args&&...args)
        {
            return f(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_FUNCTION_RVALUE
        ESTD_CPP_CONSTEXPR(14) static Result exec(void* _this, Args&&...args)
#else
        ESTD_CPP_CONSTEXPR(14) static Result exec(void* _this, Args...args)
#endif
        {
            return ((model*)_this)->operator()(std::forward<Args>(args)...);
        }
    };


    // 13MAY24 MB EXPERIMENTAL replacement for 'thisify'
    template <class T, Result (T::*f)(Args...)>
    struct method_model : model_base
    {
        constexpr explicit method_model(T* t) :
            model_base(
                static_cast<typename model_base::function_type>(&method_model::exec),
                nullptr),
            object_{t}
        {}

        T* const object_;

        constexpr Result operator()(Args&&...args) const
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_FUNCTION_RVALUE
        static Result exec(void* this_, Args&&...args)
#else
        static Result exec(void* this_, Args...args)
#endif
        {
            return (*((method_model*)this_))(std::forward<Args>(args)...);
        }
    };
};

// Special version which calls dtor right after function invocation
template <typename Result, typename... Args>
struct function_fnptr2_opt<Result(Args...)>
{
    struct model_base
    {
#if FEATURE_ESTD_FUNCTION_RVALUE
        typedef Result (*function_type)(void*, Args&&...);
#else
        typedef Result (*function_type)(void*, Args...);
#endif

        const function_type f;

        constexpr explicit model_base(function_type f) : f(f) {}

        inline Result operator()(Args&&...args)
        {
            return f(this, std::forward<Args>(args)...);
        }
    };


    template <typename F>
    struct model_void : model_base
    {
        using base_type = model_base;
        using typename base_type::function_type;

        //template <typename U>
        constexpr explicit model_void(F&& u) :
            base_type(
                static_cast<function_type>(&model_void::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

#if FEATURE_ESTD_FUNCTION_RVALUE
        static void exec(void* this_, Args&&...args)
#else
        static void exec(void* this_, Args...args)
#endif
        {
            F& f = ((model_void*)this_)->f;
            f(std::forward<Args>(args)...);
            f.~F();
        }
    };

    template <typename F>
    struct model_nonvoid : model_base
    {
        using base_type = model_base;
        using typename base_type::function_type;

        //template <typename U>
        constexpr explicit model_nonvoid(F&& u) :
            base_type(
                static_cast<function_type>(&model_nonvoid::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        static Result exec(void* this_, Args&&...args)
        {
            F& f = ((model_nonvoid*)this_)->f;
            Result r = f(std::forward<Args>(args)...);
            f.~F();
            return r;
        }
    };

    template <class F>
    using model = conditional_t<is_void<Result>::value,
        model_void<F>,
        model_nonvoid<F> >;
};

}}}

#pragma pop_macro("F")

