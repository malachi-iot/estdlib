#pragma once

#include <estd/optional.h>

namespace estd { namespace experimental {


// Moved here from internal/dynamic_array.h
// A memory lock/unlock oriented take on handles
#ifdef UNUSED_CODE
template <class T, class TAllocator>
class unique_handle : protected typed_handle<T, TAllocator>
{
    typedef typed_handle<T, TAllocator> base_t;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::allocator_traits_t allocator_traits_t;
    typedef T value_type;

    TAllocator a;

public:
    unique_handle(TAllocator& a) :
        base_t(allocator_traits_t::allocate(a, sizeof(value_type))),
        a(a) {}

    unique_handle() :
       // some allocators are not stateful
        base_t(allocator_traits_t::allocate(a, sizeof(value_type)))
    {
        static_assert(sizeof(TAllocator) == 0, "Only stateless allocator permitted here");
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    unique_handle(unique_handle&& uh) :
        base_t(uh.handle),
        a(uh.a)
    {

    }
#endif

    T& lock() { return base_t::lock(a); }
    void unlock() { return base_t::unlock(a); }
};


#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
template <class T, class TAllocator, class... TArgs>
unique_handle<T, TAllocator> make_unique_handle(TAllocator& a, TArgs&&...args)
{
    unique_handle<T, TAllocator> uh(a);

    T& val = uh.lock();

    allocator_traits<TAllocator>::construct(a, &val, args...);

    uh.unlock();

    return uh;
}

template <class T, class TAllocator, class... TArgs>
unique_handle<T, TAllocator> make_unique_handle(TArgs&&...args)
{
    static_assert(sizeof(TAllocator) == 0, "Only stateless allocator permitted here");
    static TAllocator a;
    unique_handle<T, TAllocator> uh(a);

    T& val = uh.lock();

    allocator_traits<TAllocator>::construct(a, &val, args...);

    uh.unlock();

    return uh;
}
#endif
#endif

// as per
// https://stackoverflow.com/questions/48242171/is-it-possible-to-get-the-name-of-a-type-alias-in-c
// https://stackoverflow.com/questions/20419869/is-it-possible-to-define-an-implementation-template-specialization-as-typedef-of
// We will likely have to do wrapper classes around handles.  At first this seemed like a bummer, but I remember
// I make a habit of doing this anyway to C++-ize them (i.e. embr::lwip namespace)

template <class T>
struct unique_handle;

template <class T>
struct shared_handle;

template <class T, class TOptional = estd::optional<T> >
class unique_handle_base
{
    TOptional value_;

public:
    typedef T element_type;

    operator bool() const { return value_.has_value(); }

    const element_type& operator*() const { return value_; }
};

}}