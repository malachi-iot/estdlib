#pragma once

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace estd { namespace detail { namespace impl {


template <typename Result, typename... Args>
struct function_fnptr2<Result(Args...)>
{
    // this is a slightly less fancy more brute force approach to try to diagnose esp32
    // woes
    struct model_base
    {
        typedef Result (*function_type)(void*, Args...);
        typedef void (*deleter_type)();

        const function_type _f;
#if GITHUB_ISSUE_39_EXP
        const deleter_type _d;
#endif

        constexpr explicit model_base(
            function_type f,
            deleter_type d = nullptr) :
            _f(f)
#if GITHUB_ISSUE_39_EXP
            ,
            _d{d}
#endif
        {}

        model_base(const model_base& copy_from) = default;
        // DEBT: For some reason ESP32's default move constructor
        // doesn't initialize _f
        //concept_fnptr2(concept_fnptr2&& move_from) = default;
        constexpr model_base(model_base&& move_from) noexcept:
            _f(std::move(move_from._f))
#if GITHUB_ISSUE_39_EXP
            ,
            _d{std::move(move_from._d)}
#endif
        {}

        inline Result _exec(Args&&...args)
        {
            return _f(this, std::forward<Args>(args)...);
        }
    };

    template <typename F>
    struct model : model_base
    {
        typedef model_base base_type;

        template <typename U>
        constexpr explicit model(U&& u) :
            base_type(static_cast<typename base_type::function_type>(&model::__exec)),
            f(std::forward<U>(u))
        {
        }

        /*
        model_fnptr2(const model_fnptr2& copy_from) = default;
        //model_fnptr2(model_fnptr2&& move_from) = default;
        model_fnptr2(model_fnptr2&& move_from) :
            base_type(std::move(move_from)),
            f(std::move(move_from.f))
        {} */

        F f;

        static void dtor(void* _this)
        {
            ((model*)_this)->f.~F();
        }

        // TODO: Consolidate different models down to a model_base since they
        // all need this exec function
        Result exec(Args&&...args)
        {
            return f(std::forward<Args>(args)...);
        }

        static Result __exec(void* _this, Args...args)
        {
            auto __this = ((model*)_this);

            return __this->f(std::forward<Args>(args)...);
        }
    };
};

// Special version which calls dtor right after function invocation
template <typename Result, typename... Args>
struct function_fnptr2_opt<Result(Args...)>
{
    struct model_base
    {
        typedef Result (*function_type)(void*, Args...);

        const function_type f;

        constexpr explicit model_base(function_type f) : f(f) {}

        inline Result _exec(Args&&...args)
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

        static void exec(void* this_, Args...args)
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

        static Result exec(void* this_, Args...args)
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

#endif