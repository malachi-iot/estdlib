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

// At this time, shared_handle is almost just a syntactic helper on top of unique_handle
// because we:
// a) haven't fully cracked the nut on shared_ptr, which is necessary (probably need a memory pool)
// b) when possible, will favor types which inherently already do ref counting, like pbufs - which
//    one could use directly with unique_handle since deallocate phase directly flows into ref decrement
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

template <class T>
struct shared_resource_pointer_traits
{
    typedef T element_type;
    typedef T* value_type;

    static inline void relinquish(T** v)
    {
        *v = nullptr;
    }

    static inline bool owned(const T* v)
    {
        return v != nullptr;
    }

    inline static void deleter(T* v)
    {
        v->~T();
    }
};

template <class T, class TTraits>
class shared_resource
{
protected:
    typedef TTraits resource_traits;
    typedef typename resource_traits::value_type value_type;
    typedef shared_resource this_type;

    value_type value_;

    shared_resource* next;

    bool owned() const NOEXCEPT
    {
        return resource_traits::owned(value_);
    }

    bool is_weak() const { return !owned(); }

    inline bool party_of_one() const
    {
        return next == this;
    }

    void relinquish() NOEXCEPT
    {
        resource_traits::relinquish(&value_);
    }

    void add(shared_resource* prev)
    {
        // splice ourselves into circular list
        next = prev->next;
        prev->next = this;
    }

    void add(const value_type& value, shared_resource* prev)
    {
        add(prev);

        value_ = value;
    }

    // This iterator moves forward through the linked list skipping weak pointers
    // Good const guidance from
    // https://quuxplusone.github.io/blog/2018/12/01/const-iterator-antipatterns/
    // Not going whole hog because this is an internal-only iterator
    template <bool is_const>
    class iterator_base
    {
        typedef typename estd::conditional<is_const,
            const this_type*, this_type*>::type pointer;

        pointer i;

    public:
        iterator_base(pointer i) : i{i} {}

        iterator_base& operator++()
        {
            do
            {
                i = i->next;

            // Only weak_resources are both in the list but not owned
            // shared_resources always are owned while in the list
            }   while(i->is_weak());

            return *this;
        }

        pointer operator*() const { return i; }

        bool operator!=(const iterator_base& compare_to) const
        {
            return compare_to.i != i;
        }

    };

    typedef iterator_base<false> iterator;
    typedef iterator_base<true> const_iterator;

    // Since it's a circle
    iterator end() { return iterator(this); }
    const_iterator end() const { return const_iterator(this); }

    void remove()
    {
        if(party_of_one())
        {
            // we're last one, so actually deallocate
            // DEBT: More of a traditional allocater_traits would be nice here
            // DEBT: Would it be better to do this at dtor phase?
            resource_traits::deleter(value_);
            return;
        }

        // DEBT: We'll want to bounds check, getting caught in this infinite
        // loop would be nasty.
        // DEBT: On an overall sense, we want something like FEATURE_ESTD_STRICT, but for
        // bounds checking
        shared_resource* i = next;

        // unsplice ourself out of the circular linked list
        for(; i->next != this; i = i->next);
        i->next = next;
    }

public:
    void reset() NOEXCEPT
    {
        relinquish();
        remove();
    }

    const value_type& get() const { return value_; }

    operator bool() const { return owned(); }


    int use_count() const // DEBT: Bring back const once we work out constness of iterator
    {
        if(!owned()) return 0;

        if(party_of_one()) return 1;

        int counter = 1;

        const_iterator i(this), end(this->end());

        // Skip past one, we can't do traditional begin != end because begin and end are the same
        // Additionally, we need to use the iterator's smarts to skip past a possible weak pointer
        // at the beginning
        ++i;

        for(; i != end; ++counter, ++i);
        //for(shared_resource* i = next; i != this; ++counter, i = i->next);

        return counter;
    }

    shared_resource() : next(this)  // just us
    {
        // DEBT: Optimization, when using 'optional' it will auto-init things, so no need
        // to init again here in that case
        relinquish();
    }

    template <typename Y>
    shared_resource(shared_resource<Y, resource_traits>& r)
    {
        add(r.value_, &r);
    }

    template <typename Y>
    shared_resource(shared_resource<Y, resource_traits>&& r)
    {
        add(r.value_, &r);
        r.reset();
    }

    template <typename Y>
    shared_resource(const Y& y) : next(this)
    {
        value_ = y;
    }

    ~shared_resource()
    {
        if(owned()) remove();
    }
};

template <class T, class TTraits>
class weak_resource : protected shared_resource<T, TTraits>
{
    typedef shared_resource<T, TTraits> base_type;
    typedef shared_resource<T, TTraits> shared_type;
    typedef typename base_type::iterator iterator;

    shared_type* first_shared() const
    {
        iterator i = base_type::next;

        ++i;

        return *i == this ? nullptr : *i;
    }

public:
    bool expired() const
    {
        return first_shared() != nullptr;
    }

    weak_resource() NOEXCEPT {}

    weak_resource(shared_resource<T, TTraits>& r) NOEXCEPT
    {
        base_type::add(&r);
    }

    // Not const because we ARE our own control block also
    shared_type lock() NOEXCEPT
    {
        shared_type* first = first_shared();

        return first == nullptr ? shared_type{} : shared_type(*first);
    }

    // By nature we are already relinquished, so only do a remove
    void reset() NOEXCEPT
    {
        base_type::remove();
    }
};

}}