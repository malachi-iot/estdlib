#pragma once

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace estd { namespace detail { namespace impl {


template <typename TResult, typename... TArgs>
struct function_fnptr2<TResult(TArgs...)>
{
    // this is a slightly less fancy more brute force approach to try to diagnose esp32
    // woes
    struct model_base
    {
        typedef TResult (*function_type)(void*, TArgs...);
        typedef void (*deleter_type)();

        const function_type _f;
        const deleter_type _d;

        constexpr explicit model_base(
            function_type f,
            deleter_type d = nullptr) :
            _f(f),
            _d{d}
        {}

        model_base(const model_base& copy_from) = default;
        // DEBT: For some reason ESP32's default move constructor
        // doesn't initialize _f
        //concept_fnptr2(concept_fnptr2&& move_from) = default;
        constexpr model_base(model_base&& move_from) noexcept:
            _f(std::move(move_from._f)),
            _d{std::move(move_from._d)}
        {}

        inline TResult _exec(TArgs&&...args)
        {
            return _f(this, std::forward<TArgs>(args)...);
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
        TResult exec(TArgs&&...args)
        {
            return f(std::forward<TArgs>(args)...);
        }

        static TResult __exec(void* _this, TArgs...args)
        {
            auto __this = ((model*)_this);

            return __this->f(std::forward<TArgs>(args)...);
        }
    };
};


}}}

#endif