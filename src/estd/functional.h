/**
 *
 * References:
 *
 * 1. https://wiki.sei.cmu.edu/confluence/display/cplusplus/OOP52-CPP.+Do+not+delete+a+polymorphic+object+without+a+virtual+destructor
 */
#pragma once

#include "internal/platform.h"
#include "internal/functional.h"
#include "internal/value_evaporator.h"

#include "tuple.h"
#include "type_traits.h"
#include "traits/allocator_traits.h"

#include "internal/invoke.h"

#include "internal/raw/variant.h"

#include "exp/functional.h"
#include "obsolete/functional.h"

// TODO: Utilize std version of this, if available

namespace estd {

template <class T>
class reference_wrapper {
public:
    // types
    typedef T type;

    // construct/copy/destroy
    reference_wrapper(T& ref) NOEXCEPT : _ptr(estd::addressof(ref)) {}
#ifdef FEATURE_CPP_MOVESEMANTIC
    reference_wrapper(T&&) = delete;
#endif
#ifdef FEATURE_CPP_DEFAULT_CTOR
    reference_wrapper(const reference_wrapper&) NOEXCEPT = default;

    // assignment
    reference_wrapper& operator=(const reference_wrapper& x) NOEXCEPT = default;
#else
    reference_wrapper(const reference_wrapper& copy_from) NOEXCEPT :
    _ptr(copy_from._ptr)
  {};

  // assignment
  reference_wrapper& operator=(const reference_wrapper& copy_from) NOEXCEPT
  {
    _ptr = copy_from._ptr;
  }
#endif

    // access
    operator T& () const NOEXCEPT { return *_ptr; }
    T& get() const NOEXCEPT { return *_ptr; }

    /*
    template< class... ArgTypes >
    estd::invoke_result_t<T&, ArgTypes...>
      operator() ( ArgTypes&&... args ) const {
      return estd::invoke(get(), std::forward<ArgTypes>(args)...);
    } */

private:
    T* _ptr;
};


#ifdef FEATURE_CPP_DEDUCTION_GUIDES
// deduction guides
template<class T>
reference_wrapper(reference_wrapper<T>) -> reference_wrapper<T>;
#endif

template <class T>
reference_wrapper<T> ref(T& t) NOEXCEPT
{
    return reference_wrapper<T>(t);
}

template <class T>
reference_wrapper<T> cref(const T& t) NOEXCEPT
{
    return reference_wrapper<T>(t);
}

#if defined (FEATURE_CPP_VARIADIC) && defined (FEATURE_CPP_MOVESEMANTIC)

namespace experimental {

// Guidance from
// https://stackoverflow.com/questions/14936539/how-stdfunction-works

template <typename T, bool nullable = true, class TAllocator = monostate>
class function;

template <class TFunc, typename F>
class inline_function;


template <typename TResult, typename... TArgs, bool nullable, class TAllocator>
class function<TResult(TArgs...), nullable, TAllocator> :
    public detail::function<TResult(TArgs...)>,
    //public function_base_tag,
    public estd::internal::reference_evaporator < TAllocator, false> //estd::is_class<TAllocator>::value>
{
    typedef detail::function<TResult(TArgs...)> base_type;
    using typename base_type::model_base;

    typedef estd::internal::reference_evaporator <TAllocator, false> allocator_provider_type;
    typedef estd::allocator_traits<TAllocator> allocator_traits;

    /*
    template <typename F, typename TAllocator>
    struct model_with_allocator : model<F>
    {
        TAllocator& alloc;

        model_with_allocator(F&& f, TAllocator& alloc) :
            model<F>(std::forward<F>(f)),
            alloc(alloc)
        {

        }
    }; */

private:
    model_base* getm() const { return base_type::m; }

    void clear()
    {
        base_type::m = nullptr;
    }

    typedef void (*deleter_type)(model_base*);  // EXPERIMENTAL

    template <class F>
    static void deleter(model_base* c)
    {
        typedef typename estd::decay_t<F> incoming_function_type;
        typedef typename base_type::template model<incoming_function_type> model_type;

        delete (model_type*) c;
    }

    // Jumping through some hoops to avoid overhead of a virtual destructor.  On a plus note
    const deleter_type _deleter;

    void reset()
    {
        // DEBT: function_base does not check for nullptr before () is called
        if(nullable == false || base_type::m != nullptr)
            // FIX: Don't want to dynamically allocate memory quite this way
            // DEBT: Technically we aren't deleting `concept` but instead our
            // special subclass `model_type` (see ctor) - this may not delete properly
            // i.e. may not delete the extended memory that `model_type` occupies
            // a virtual destructor may be necessary for that.  We're careful to (so far)
            // not need a virtual destructor otherwise.
            //delete base_type::m;

            _deleter(base_type::m);
    }

public:
    /*
    template <typename T,
        estd::enable_if_t<!estd::is_base_of<function_base_tag,
            estd::remove_cv_t<T>
            >::value, bool> = true> */
    template <typename T>
    function(T&& t) :
        allocator_provider_type(TAllocator()),
        _deleter{deleter<T>}
    {
        typedef typename estd::decay<T>::type incoming_function_type;
        typedef typename base_type::template model<incoming_function_type> model_type;
        //allocator_traits::rebind_alloc<model_type>
        // FIX: Don't want to dynamically allocate memory quite this way
        auto m = new model_type(std::move(t));
        base_type::m = m;
    }

    template <typename T>
    function(T&& t, TAllocator& allocator) :
        allocator_provider_type(allocator),
        _deleter{deleter<T>}
    {
        typedef typename estd::decay<T>::type incoming_function_type;
        typedef typename base_type::template model<incoming_function_type> model_type;
        auto m = new model_type(std::forward<T>(t));
        base_type::m = m;
    }

    /*
     * Aborting this, thinking estd::function itself is better off taking TAllocator
     * so that 'delete' free operation takes place at 'function' level as it should
     * rather than model level which could be problematic if for example a subclass
     * needed to do more things on destruction.  It also puts off the need for a
     * virtual destructor
    template <typename F, typename TAllocator>
    function(F&& f, TAllocator& alloc)
    {
        typedef estd::allocator_traits<TAllocator> allocator_traits;
        typedef model_with_allocator<typename std::decay<F>::type, TAllocator> model_type;

        auto h = allocator_traits::allocate(alloc, sizeof(model_type));
    } */

    function(const function& other) = default;

#ifdef FEATURE_CPP_MOVESEMANTIC
    // FIX: SFINAE remove this when nullable == false
    function(function&& other) NOEXCEPT :
        allocator_provider_type(std::move(other)),
        _deleter{other._deleter}
    {
        base_type::m = other.getm();
        other.clear();
    }
#endif

    ~function()
    {
        reset();
    }

    template <typename F>
    function& operator=(F&& f)
    {
        reset();
        new (this) function(std::move(f));
        return *this;
    }

    template <typename F>
    static inline_function<F, TResult(TArgs...)> make_inline2(F&& f)
    {
        return inline_function<F, TResult(TArgs...)>(std::move(f));
    }

    explicit operator bool() const noexcept
    {
        return nullable ? base_type::m != nullptr : true;
    }
};

// Guidance from
// https://stackoverflow.com/questions/39131137/function-pointer-as-template-argument-and-signature

template <typename TFunc, TFunc f>
class context_function;

template <typename TResult, typename... TArgs, class TContext, TResult (TContext::*f)(TArgs...)>
class context_function<TResult(TContext::*)(TArgs...), f> :
    public internal::thisify_function<TResult(TArgs...)>
{
    typedef internal::thisify_function<TResult(TArgs...)> base_type;

    typedef typename base_type::template model<TContext, f> model_type;

public:
    context_function(TContext* foreign_this) :
        base_type(model_type(foreign_this))
    {

    }
};



template <class TFunc, typename TResult, typename... TArgs>
class inline_function<TFunc, TResult(TArgs...)> : public detail::function<TResult(TArgs...)>
{
    typedef detail::function<TResult(TArgs...)> base_type;
    typedef typename base_type::template model<TFunc> model_type;

    model_type m;

public:
    inline_function(TFunc&& f) :
        base_type(&m),
        m(std::move(f))
    {

    }

    /*
    inline_function(const inline_function& copy_from) = default;
    inline_function(inline_function&& move_from) :
        base_type(std::move(move_from)),
        m(std::move(move_from.m))
    {

    } */
};

template <class TFunc, typename F>
inline_function<TFunc, F> make_inline_function(F&& f)
{
    return inline_function<TFunc, F>();
}

//namespace internal {

template <class TFunc>
struct bind_helper;

template <typename TResult, class ...TArgs>
struct bind_helper<TResult(TArgs...)>
{
    using R = TResult;
    using A = estd::tuple<TArgs...>;
};

template <class TFunc, class F, class ...TArgs>
struct bind_helper2;

template <typename TResult, class ...TArgs2, class F, class ...TArgs>
struct bind_helper2<TResult(TArgs2...), F, TArgs...>
{
};

// TODO: Put into an internal area
template <class TFunc, class F, class ...TArgs>
struct bind_type : inline_function<TFunc, F>
{
    typedef inline_function<TFunc, F> base_type;

    tuple<TArgs...> args;

    typedef typename base_type::template model<F> model_type;

    model_type m;

    bind_type(F&& f, TArgs&&...args) :
        base_type(std::move(f)),
        args(std::forward<TArgs>(args)...)
    {

    }

    template <typename ...TArgs2>
    auto operator ()(TArgs2&&...args2) -> decltype (apply(std::move(base_type::f), std::move(args)))
    {
        return apply(std::move(base_type::f), std::move(args));
    }
};

//}

template <class F, class ...TArgs>
auto bind(F&& f, TArgs&&... args) -> bind_type<decltype(f), F, TArgs...>
{
    bind_type<decltype(f), F, TArgs...> b(
        std::move(f),
        std::forward<TArgs>(args)...
    );

    return b;
}



}

#endif  // VARIADIC + MOVE SEMANTIC

}
