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

#include "variant.h"

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


#if defined (FEATURE_CPP_VARIADIC) && defined (FEATURE_CPP_MOVESEMANTIC)

namespace obsolete {

template <class F>
struct function
{
    F f;

    function() NOEXCEPT = default;

    template <class F2>
    function(F2 f) : f(f) {}

    // deviates from spec here, but doing this to conform with how bind likes to
    // do things
    function(F&& f) : f(std::move(f)) {}

    // deviates from spec, function class itself is supposed to take these args
    template <class ...TArgs>
    auto operator()(TArgs&&...args) -> decltype (f(args...))
    {
        return f(std::forward<TArgs>(args)...);
    }

    // UNTESTED
    function& operator=(F&& f)
    {
        this->f = std::move(f);
        return *this;
    }

    function& operator=(reference_wrapper<F> f) NOEXCEPT
    {
        this->f = f;
        return *this;
    }
};

namespace internal {


template <class F, class ...TArgs>
struct bind_type : function<F>
{
    typedef function<F> base_type;

    tuple<TArgs...> args;

    bind_type(F&& f, TArgs&&...args) :
        base_type(std::move(f)),
        args(std::forward<TArgs>(args)...)
    {

    }

    auto operator ()() -> decltype (apply(std::move(base_type::f), std::move(args)))
    {
        return apply(std::move(base_type::f), std::move(args));
    }
};

}

template <class F, class ...TArgs>
auto bind(F&& f, TArgs&&... args) -> internal::bind_type<F, TArgs...>
{
    internal::bind_type<F, TArgs...> b(
                std::move(f),
                std::forward<TArgs>(args)...
                );

    return b;
}

}

#endif




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

template <typename F, typename TResult, typename... TArgs>
struct model_base
{

};

template <typename T, bool nullable = true, class TAllocator = monostate>
class function;

template <typename TResult, typename... TArgs>
class function_base;

struct function_base_tag {};


template <typename TResult, typename... TArgs>
class function_base<TResult(TArgs...)> : public function_base_tag
{
protected:
    // function pointer approach works, but if we have to add in a virtual destructor
    // (which is likely) then we are faced with incurring that overhead anyway so might
    // switch over to virtual operator() in that case
    struct concept_fnptr1
    {
        typedef TResult (concept_fnptr1::*function_type)(TArgs&&...);
        typedef void (*deleter_type)(concept_fnptr1*);  // EXPERIMENTAL

        const function_type f;

        concept_fnptr1(function_type f) : f(f) {}

        concept_fnptr1(const concept_fnptr1& copy_from) = default;
        // just like concept_fnptr2, default move constructor somehow
        // results in make_inline2 leaving f uninitialized
        //concept_fnptr1(concept_fnptr1&& move_from) = default;
        concept_fnptr1(concept_fnptr1&& move_from) :
            f(move_from.f)
        {}

        inline TResult _exec(TArgs&&...args)
        {
            return (this->*f)(std::forward<TArgs>(args)...);
        }
    };


    template <typename F>
    struct model_fnptr1 : concept_fnptr1
    {
        typedef concept_fnptr1 base_type;

        //template <typename U>
        model_fnptr1(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model_fnptr1::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        TResult exec(TArgs&&...args)
        {
            return f(std::forward<TArgs>(args)...);
        }
    };

    // this is a slightly less fancy more brute force approach to try to diagose esp32
    // woes
    struct concept_fnptr2
    {
        typedef TResult (*function_type)(void*, TArgs&&...);

        const function_type _f;

        concept_fnptr2(function_type f) : _f(f) {}

        concept_fnptr2(const concept_fnptr2& copy_from) = default;
        // DEBT: For some reason ESP32's default move constructor
        // doesn't initialize _f
        //concept_fnptr2(concept_fnptr2&& move_from) = default;
        concept_fnptr2(concept_fnptr2&& move_from) :
            _f(std::move(move_from._f))
        {}

        inline TResult _exec(TArgs&&...args)
        {
            return _f(this, std::forward<TArgs>(args)...);
        }
    };

    template <typename F>
    struct model_fnptr2 : concept_fnptr2
    {
        typedef concept_fnptr2 base_type;

        model_fnptr2(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model_fnptr2::__exec)),
            f(std::forward<F>(u))
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

        // TODO: Consolidate different models down to a model_base since they
        // all need this exec function
        TResult exec(TArgs&&...args)
        {
            return f(std::forward<TArgs>(args)...);
        }

        static TResult __exec(void* _this, TArgs&&...args)
        {
            auto __this = ((model_fnptr2*)_this);

            return __this->f(std::forward<TArgs>(args)...);
        }
    };

    struct concept_virtual
    {
        virtual TResult _exec(TArgs&&...args) = 0;
        virtual ~concept_virtual() = default;
    };

    template <class F>
    struct model_virtual : concept_virtual
    {
        model_virtual(F&& u) :
            f(std::forward<F>(u))
        {
        }

        F f;

        virtual TResult _exec(TArgs&&...args) override
        {
            return f(std::forward<TArgs>(args)...);
        }

        inline TResult exec(TArgs&&...args)
        {
            return f(std::forward<TArgs>(args)...);
        }
    };

    // DEBT: Need to change name from concept, I suspect this will cause
    // c++20 issues
    // We like this way best, but due to [1] it may not be viable.
    // However, they say that:
    // "Deleting a polymorphic object without a virtual destructor
    //  is permitted if the object is referenced by a pointer to its
    //  class, rather than via a pointer to a class it inherits from."
    // This might be possible with a manual function pointer deleter.
    typedef concept_fnptr1 concept;
    template <class F>
    using model = model_fnptr1<F>;

    /*
    typedef concept_fnptr2 concept;
    template <class F>
    using model = model_fnptr2<F>; */

    /*
    typedef concept_virtual concept;
    template <class F>
    using model = model_virtual<F>; */

    // DEBT: 'function' constructors need this to not be const, for now
    //concept* const m;
    concept* m;

protected:
    //function_base(function_type f) : f(f) {}

public:
    function_base() : m(NULLPTR) {}

    function_base(concept* m) : m(m) {}

    function_base(const function_base& copy_from) = default;

    // NOTE: If we decide to add nullability to function_base, then a move constructor
    // makes sense
    function_base(function_base&& move_from) = delete;

    function_base& operator =(const function_base&) = default;

#if __cplusplus >= 201402L
    TResult operator()(TArgs&&... args)
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
#endif

    // DEBT: Unknown why STM32 flavor doesn't compile this.  Starting to seem like different versions
    // of c++ handle this particular overload in different ways.  Technically we prefer the
    // above version solo anyway.
#if !defined(ESTD_MCU_STM32)
    // FIX: Unsure why we need both this and && version, but somehow forwarding an
    // lvalue with TArgs&& in this context makes it mad
    TResult operator()(TArgs... args)
    {
        return m->_exec(std::forward<TArgs>(args)...);
    }
#endif

    explicit operator bool() const NOEXCEPT { return m != NULLPTR; }

    template <typename F>
    static model<F> make_inline(F&& f)
    {
        return model<F>(std::move(f));
    }

    // EXPERIMENTAL
    const concept* getm() const { return m; }
};

template <class TFunc, typename F>
class inline_function;


template <typename TResult, typename... TArgs, bool nullable, class TAllocator>
class function<TResult(TArgs...), nullable, TAllocator> :
    public function_base<TResult(TArgs...)>,
    //public function_base_tag,
    public estd::internal::reference_evaporator < TAllocator, false> //estd::is_class<TAllocator>::value>
{
    typedef function_base<TResult(TArgs...)> base_type;
    typedef typename base_type::concept concept;

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
    concept* getm() const { return base_type::m; }

    void clear()
    {
        base_type::m = nullptr;
    }

    typedef void (*deleter_type)(concept*);  // EXPERIMENTAL

    template <class F>
    static void deleter(concept* c)
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

template <typename TFunc>
class context_function;

// DEBT: Only works with 'method 1' concept/model at the moment
template <typename TResult, typename... TArgs>
class context_function<TResult(TArgs...)> : public function_base<TResult(TArgs...)>
{
    typedef function_base<TResult(TArgs...)> base_type;
    typedef typename base_type::concept concept_base;
    typedef context_function this_type;

    //typedef TResult (concept_fnptr1::*function_type)(TArgs&&...);
    //typedef typename concept::function_type function_type;

protected:
    template <class T>
    using function_type = TResult (T::*)(TArgs...);

public:
    template <class T, function_type<T> f>
    struct model : concept_base
    {
        typedef concept_base base_type;

        T* const foreign_this;

        // NOTE: This is a bizarre thing we do here.  We take advantage of the fact that pointer
        // sizes don't change and accept a foreign-typed model.  We do this to simulate a runtime
        // templated union initialization.  base type gets initialized with a constant pointer to
        // foreign model's exec helper, and naturally we copy over the 'this'.  I would not be
        // surprised if this falls into "undefined" behavior at some point, but in the end we are
        // only relying on 2 runtime pointers to not change size and 2 compile time pointers
        template <class T2, function_type<T2> f2>
        constexpr model(const model<T2, f2>& copy_from) :
            base_type(static_cast<typename base_type::function_type>(&model<T2, f2>::exec)),
            foreign_this{(T*)copy_from.foreign_this}
        {
        }

        model(T* foreign_this) :
            base_type(static_cast<typename base_type::function_type>(&model::exec)),
            foreign_this{foreign_this}
        {

        }

        TResult exec(TArgs&&...args)
        {
            return (foreign_this->*f)(std::forward<TArgs>(args)...);
        }
    };

    /* Nifty but not useful here
    template <class T, function_type<T> f>
    union holder
    {
        model<T, f> m;
    };*/

    TResult dummy(TArgs...args) { return TResult{}; }

    model<this_type, &this_type::dummy> m_;

public:
    /*
    template <class T, function_type<T> f>
    context_function(T* foreign_this)
    {
    } */

    template <class T, function_type<T> f>
    constexpr context_function(model<T, f> m) :
        base_type(&m_),
        m_(m)
    {
    }

    template <class T, function_type<T> f>
    static context_function create(T* foreign_this)
    {
        return context_function(model<T, f>(foreign_this));
    }
};

// Guidance from
// https://stackoverflow.com/questions/39131137/function-pointer-as-template-argument-and-signature

template <typename TFunc, TFunc f>
class context_function2;

template <typename TResult, typename... TArgs, class TContext, TResult (TContext::*f)(TArgs...)>
class context_function2<TResult(TContext::*)(TArgs...), f> :
    public context_function<TResult(TArgs...)>
{
    typedef context_function<TResult(TArgs...)> base_type;

    typedef typename base_type::template function_type<TContext> function_type;
    typedef typename base_type::template model<TContext, f> model_type;

public:
    context_function2(TContext* foreign_this) :
        base_type(model_type(foreign_this))
    {

    }
};



template <class TFunc, typename TResult, typename... TArgs>
class inline_function<TFunc, TResult(TArgs...)> : public function_base<TResult(TArgs...)>
{
    typedef function_base<TResult(TArgs...)> base_type;
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
