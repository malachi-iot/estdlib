#pragma once

#include "../memory.h"

namespace estd {

namespace experimental {
#ifdef UNUSED
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

}





namespace experimental {


template <class TAllocator>
class memory_range_base
{
public:
    typedef TAllocator allocator_t;

    typedef typename allocator_t::handle_type handle_type;
    typedef typename allocator_t::pointer pointer;

private:
    //allocator_t
    handle_type handle;

public:
    template <class T>
    T& lock()
    {
        //pointer p =
    }
};


// tracks handle, capacity (via handle_with_size) and
// "used" size with up to 3 distinctive variables
// also tracks allocator itself
template <class TAllocator, class TAllocatorInstance = TAllocator&>
struct size_tracker_default
{
    typedef TAllocator allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_traits<TAllocator>::size_type size_type;

    handle_with_size handle;
    size_type m_size;
    TAllocatorInstance allocator;

    size_type capacity() const { return allocator.size(handle); }
    size_type size() const { return m_size; }

    size_tracker_default() {}

    size_tracker_default(allocator_type& a) : allocator(a) {}
};


// tracks handle and capacity (via handle_with_size) with up to 2 distinctive variables,
// and ascertains size by doing null termination search
template <class TAllocator, class TAllocatorInstance = TAllocator&>
struct size_tracker_nullterm
{
    typedef TAllocator allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_traits<TAllocator>::size_type size_type;

    handle_with_size handle;
    TAllocatorInstance allocator;

    size_type capacity() const { return allocator.size(handle); }
    size_type size()
    {
        value_type* s = &handle.lock();

        size_type len = ::strlen(s);

        handle.unlock();
    }

    //size_tracker_nullterm() : allocator(TAllocator()) {}

    size_tracker_nullterm(const TAllocator& a) : allocator(a) {}
};


// TODO: come up with better name, specialization like traits except stateful to
// track a singular allocation within an allocator.  Revision of above size_tracker_nullterm
// and size_tracker_default
template <class TAllocator>
class dynamic_array_helper
{
    // default implementation is 'full fat' to handle all scenarios
    typedef TAllocator allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_traits<TAllocator>::size_type size_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

    // handle.size represents currently allocation portion
    handle_with_size handle;
    // remember, size represents 'user/app' portion.
    size_type m_size;
    // don't fiddle with ref juggling here - if that's absolutely necessary use
    // the RefAllocator helper
    allocator_type allocator;

public:
    size_type capacity() const { return allocator.size(handle); }
    size_type size() const { return m_size; }

    allocator_type& get_allocator() const { return allocator; }

    // +++ intermediate calls, phase these out eventually
    handle_with_size get_handle() { return handle; }
    void size(size_type s) { m_size = s; }
    // ---

    handle_with_offset offset(size_type pos)
    {
        return allocator.offset(handle, pos);
    }

    value_type& lock() { return allocator.lock(handle); }
    void unlock() { allocator.unlock(handle); }

    void allocate(size_type capacity)
    {
        handle = allocator.allocate_ext(capacity);
    }


    void reallocate(size_type capacity)
    {
        handle = allocator.reallocate_ext(handle, capacity);
    }

    bool is_allocated() const
    {
        return handle != allocator_type::invalid();
    }

    template <class T>
    dynamic_array_helper(T init) :
    //allocator(init),
            handle(allocator_type::invalid()),
            m_size(0)
    {

    }

    ~dynamic_array_helper()
    {
        if(handle != allocator_type::invalid())
            allocator.deallocate(handle);
    }
};


// non standard base class for managing expanding/contracting arrays
// accounts for lock/unlock behaviors. Used for vector and string
// More or less 1:1 with vector
// and may get rolled back completely into vector at some point -
// size_tracker_* are very experimental
template <class TAllocator, class THelper = dynamic_array_helper<TAllocator>>
class dynamic_array
{
public:
    typedef TAllocator allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_traits<TAllocator>::size_type size_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

protected:
    typename allocator_type::lock_counter lock_counter;

    THelper helper;

    handle_with_offset offset(size_type  pos)
    {
        return helper.offset(pos);
    }

    value_type* lock()
    {
        lock_counter++;
        return &helper.lock();
    }

    void unlock()
    {
        lock_counter--;
        helper.unlock();
    }

    // internal method for auto increasing capacity based on pre-set amount
    void ensure_additional_capacity(size_type increase_by)
    {
        size_type cap = capacity();

        // TODO: assert increase_by is a sensible value
        // above 0 and less than ... something

        if(size() + increase_by >= cap)
        {
            // increase by as near to 32 bytes as is practical
            reserve(cap + increase_by + ((32 + sizeof(value_type)) / sizeof(value_type)));
        }
    }


    void raw_insert(value_type* a, value_type* to_insert_pos, const value_type* to_insert_value)
    {
        // NOTE: may not be very efficient (underlying allocator may need to realloc/copy etc.
        // so later consider doing the insert operation at that level)
        ensure_additional_capacity(1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = (((to_insert_pos) - a) / sizeof(value_type));
        size_type remaining = size() - raw_typed_pos;

        memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));
        *to_insert_pos = *to_insert_value;

        helper.size(helper.size() + 1);
    }

public:
    dynamic_array() :
            helper(NULLPTR)
    {}

    allocator_type get_allocator() const
    {
        return helper.get_allocator();
    }

    size_type size() const { return helper.size(); }

    size_type capacity() const
    {
        return helper.capacity();
    }

    void reserve( size_type new_cap )
    {
        if(helper.is_allocated())
            helper.allocate(new_cap);
        else
            helper.reallocate(new_cap);
    }

    void push_back(const value_type& value)
    {
        ensure_additional_capacity(1);

        value_type* v = lock();

        v[size()] = value;

        unlock();

        helper.size(helper.size() + 1);
    }

protected:
    void _append(const value_type* buf, size_type len)
    {
        ensure_additional_capacity(len);

        value_type* raw = lock();

        memcpy(raw + size(), buf, len);

        unlock();

        helper.size(size() + len);
    }
};


}

}