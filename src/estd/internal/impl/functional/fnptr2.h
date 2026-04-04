#pragma once

#include "../../type_traits.h"
#include "../../rtto.h"

#include "fwd.h"

// Arduino compatibility
#pragma push_macro("F")
#undef F

namespace estd { namespace detail { namespace impl {

// DORMANT: To be used by all flavors of function_fnptr2
template <typename Result, typename... Args>
struct function_fnptr2_model
{
    // Effectively a functor itself wrapping our true functor - adding mainly 'exec' and of
    // course bringing in whatever Base things we want to prepend
    template <class F, class Base>
    class model : public Base
    {
        //static_assert(estd::is_base_of<base, Base>::value, "Must derive from 'base'");

        F functor_;

    public:
        constexpr explicit model(F&& f) :
            functor_{std::forward<F>(f)}
        {}

        // DEBT: Consider https://github.com/malachi-iot/estdlib/issues/186 - may affect
        // signature here
        ESTD_CPP_CONSTEXPR(14) Result operator()(Args&&...args)
        {
            return functor_(std::forward<Args>(args)...);
        }

        ESTD_CPP_DEFAULT_RULE_OF_5(model);

#if FEATURE_ESTD_FUNCTION_RVALUE
        ESTD_CPP_CONSTEXPR(14) static Result exec(void* _this, Args&&...args)
#else
        ESTD_CPP_CONSTEXPR(14) static Result exec(void* _this, Args...args)
#endif
        {
            return ((model*)_this)->operator()(std::forward<Args>(args)...);    // NOLINT
        }
    };
};

template <typename Result, typename... Args, fn_options o>
struct function_fnptr2<Result(Args...), o> : public internal::rtto_base
{
    template <class Derived>
    struct mixin
    {
        Result invoke(Args&&... args)
        {
            auto self = static_cast<Derived*>(this);

            return self->_invoke(this, std::forward<Args>(args)...);
        }

        Result operator()(Args&&... args)
        {
            return invoke(std::forward<Args>(args)...);
        }
    };

    class base2
    {
        template <class Derived>
        friend struct mixin;
    protected:
#if FEATURE_ESTD_FUNCTION_RVALUE
        using function_type = Result (*)(void*, Args&&...);
#else
        using function_type = Result (*)(void*, Args...);
#endif

        const function_type fptr_;

        constexpr explicit base2(function_type f) :
            fptr_(f)
        {}

        // Prepending _ because normally nobody should be calling this guy - only mixin
        Result _invoke(void* self, Args&&... args) const
        {
            // Special treatment needed because MI creates a mismatch between 'self' and 'this'
            return fptr_(self, std::forward<Args>(args)...);
        }
    };


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

    struct model_base : model_rtto_base, base2, mixin<model_base>
    {
        constexpr explicit model_base(
            typename base2::function_type f,
            utility_type u) :
            model_rtto_base(u),
            base2(f)
        {}
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
        // all need this exec function.
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

template <typename Result, typename... Args, fn_options o>
struct function_fnptr2_trivial<Result(Args...), o>
{
    template <class F>
    class model
    {
        // Not ready, see https://github.com/malachi-iot/estdlib/issues/171
        static_assert(estd::is_trivially_constructible<F>::value, "F must be trivial");
        //static_assert(estd::is_trivially_moveable<F>::value, "F must be trivial");
    };
};

// Special version which calls dtor right after function invocation
template <typename Result, typename... Args>
struct function_fnptr2_oneshot<Result(Args...)>
{
    using base2 = typename function_fnptr2<Result(Args...)>::base2;

    template <class Derived>
    using mixin = typename function_fnptr2<Result(Args...)>::template mixin<Derived>;

    struct model_base : base2, mixin<model_base>
    {
        constexpr explicit model_base(typename base2::function_type f) : base2(f) {}
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

