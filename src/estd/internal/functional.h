/**
 * @file
 *
 * flavor of functional with minimized dependencies
 */
#pragma once

#include "fwd/functional.h"
#include "impl/functional.h"
#include "../type_traits.h"

namespace estd {

//#if __cplusplus < 201200L
template<class T>
struct less
{
    CONSTEXPR bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs < rhs;
    }
};

template <class T>
struct greater
{
    CONSTEXPR bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs > rhs;
    }
};

/*
#else
template< class T = void >
struct less;
#endif
*/



namespace experimental {

#ifdef FEATURE_CPP_ALIGN
// kind of a blend of reference_wrapper and aligned_storage
// experimental, but very useful (abusing raw_instance_provider for this right now)
// NOTE: This in theory could be 1:1 with a fixed allocator, who should be using aligned_storage too
template <class T, size_t Align = alignof(T)>
class instance_wrapper
{
    // Not sure an item all by its lonesome in a class is gonna benefit from aligned_storage
    // perhaps instead we should make it a parent?
    typename estd::aligned_storage<sizeof(T), Align>::type _value;

public:
    typedef T value_type;

    value_type& value() { return *(T*)&_value; }
    const value_type& value() const { return *(T*)&_value; }

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
    value_type& construct(TArgs&&...args)
    {
        void* loc = reinterpret_cast<void*>(&_value);
        new (loc) value_type(std::forward<TArgs>(args)...);
        return value();
    }
#else
    value_type& construct()
    {
        new (&_value) value_type();
        return value();
    }

    template <class TParam1>
    value_type& construct(TParam1 p1)
    {
        new (&_value) value_type(p1);
        return value();
    }

    template <class TParam1, class TParam2>
    value_type& construct(TParam1 p1, TParam2 p2)
    {
        new (&_value) value_type(p1, p2);
        return value();
    }
#endif

    void destroy()
    {
        value().~value_type();
    }

    operator value_type& () { return value(); }

    operator const value_type& () const { return value(); }

    value_type& operator =(const value_type& copy_from)
    {
        value() = copy_from;
        return value();
    }
};
#endif

}

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)

namespace detail {

/// Lower-level version of function which is hands-off for memory management
template <typename TResult, typename... TArgs, class TImpl>
class function<TResult(TArgs...), TImpl> : public internal::function_base_tag
{
protected:
    typedef TImpl impl_type;

    // DEBT: Don't really like model_base and model as public, but may be necessary
public:
    typedef typename impl_type::model_base model_base;

    // NOTE: c++17 CTAD may be required for this to really be useful
    // https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
    template <class F>
    using model = typename impl_type::template model<F>;

protected:
    // DEBT: 'function' constructors need this to not be const, for now
    //concept* const m;
    model_base* m;

    //function_base(function_type f) : f(f) {}

public:
    function() : m(NULLPTR) {}

    function(model_base* m) : m(m) {}

    function(const function& copy_from) = default;

    // NOTE: If we decide to add nullability to function_base, then a move constructor
    // makes sense
    function(function&& move_from) = delete;

    function& operator =(const function&) = default;

    // NOTE: Removed separate && version since it was an ambiguous overload, and
    // std::function only has this kind of signature anyway
    // https://en.cppreference.com/w/cpp/utility/functional/function/operator()
    inline TResult operator()(TArgs... args)
    {
        // a little complicated.  Some guidance from:
        // https://stackoverflow.com/questions/2402579/function-pointer-to-member-function
        // the first portion m->* indicates that a method function pointer call is happening
        // and to load in 'm' to the 'this' pointer.  The (m->f) portion actually retrieves
        // the function pointer itself
        //return (m->*(m->f))(std::forward<TArgs>(args)...);

        // DEBT: Prefer lower overhead of above mess, but while we diagnose ESP32 failures
        // let's make our lives easier
        return m->_exec(std::forward<TArgs>(args)...);
    }

    explicit operator bool() const NOEXCEPT { return m != NULLPTR; }

    // See above 'model' CTAD comments
    template <typename F>
    inline static model<F> make_inline(F&& f)
    {
        return model<F>(std::move(f));
    }

    // EXPERIMENTAL
    const model_base* getm() const { return m; }

    // EXPERIMENTAL
    template <template <typename F> class TProvided>
    using provided = TProvided<TResult(TArgs...)>;
};

}

#endif

}
