#pragma once

#include "../memory.h"

#ifdef FEATURE_CPP_INITIALIZER_LIST
#include <initializer_list>
#include <algorithm>
#endif

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

}





namespace internal {




// TODO: come up with better name, specialization like traits except stateful to
// track a singular allocation within an allocator.  Revision of above size_tracker_nullterm
// and size_tracker_default
template <class TAllocator>
class dynamic_array_helper
{
    // default implementation is 'full fat' to handle all scenarios
    typedef TAllocator allocator_type;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    // handle.size represents currently allocation portion
    handle_with_size handle;
    // remember, size represents 'user/app' portion.
    size_type m_size;
    // don't fiddle with ref juggling here - if that's absolutely necessary use
    // the RefAllocator helper
    allocator_type allocator;

public:
    static CONSTEXPR bool uses_termination() { return false; }

    size_type capacity() const { return allocator.size(handle); }
    size_type size() const { return m_size; }

    allocator_type& get_allocator() { return allocator; }

    // +++ intermediate calls, phase these out eventually
    handle_with_size get_handle() { return handle; }
    void size(size_type s) { m_size = s; }
    // ---

    handle_with_offset offset(size_type pos)
    {
        return allocator.offset(handle, pos);
    }

    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return allocator_traits::lock(allocator, handle, pos, count);
    }

    // constant-return-lock
    const value_type& clock_experimental(size_type pos = 0, size_type count = 0)
    {
        return allocator.clock_experimental(handle, pos, count);
    }

    void unlock()
    {
        allocator_traits::unlock(allocator, handle);
    }

    bool is_allocated() const
    {
        handle_type h = handle;
        return h != allocator_traits::invalid();
    }

    bool allocate(size_type capacity)
    {
        handle = allocator.allocate_ext(capacity);
        return is_allocated();
    }


    bool reallocate(size_type capacity)
    {
        handle = allocator.reallocate_ext(handle, capacity);
        return is_allocated();
    }

    template <class T>
    dynamic_array_helper(T init) :
            allocator(init),
            handle(allocator_traits::invalid()),
            m_size(0)
    {

    }

    dynamic_array_helper() :
            handle(allocator_traits::invalid()),
            m_size(0)
    {

    }

    ~dynamic_array_helper()
    {
        if(handle != allocator_traits::invalid())
            allocator.deallocate(handle, 1);
    }
};


// non standard base class for managing expanding/contracting arrays
// accounts for lock/unlock behaviors. Used for vector and string
// More or less 1:1 with vector
// and may get rolled back completely into vector at some point -
// size_tracker_* are very experimental
template <class TAllocator, class THelper = dynamic_array_helper<TAllocator > >
class dynamic_array
{
public:
    typedef TAllocator allocator_type;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef THelper helper_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    // TODO: utilize SFINAE if we can
    // ala https://stackoverflow.com/questions/7834226/detecting-typedef-at-compile-time-template-metaprogramming
    //typedef typename allocator_type::accessor accessor_experimental;

protected:
#ifdef FEATURE_ESTD_LOCK_COUNTER
    typename allocator_traits::lock_counter lock_counter;
#endif

    THelper helper;

    handle_with_offset offset(size_type  pos)
    {
        return helper.offset(pos);
    }

    value_type* lock(size_type pos = 0, size_type count = 0)
    {
#ifdef FEATURE_ESTD_LOCK_COUNTER
        lock_counter++;
#endif
        return &helper.lock(pos, count);
    }

    const value_type* clock_experimental(size_type pos = 0, size_type count = 0)
    {
#ifdef FEATURE_ESTD_LOCK_COUNTER
        lock_counter++;
#endif
        return &helper.clock_experimental(pos, count);
    }

    void unlock()
    {
#ifdef FEATURE_ESTD_LOCK_COUNTER
        lock_counter--;
#endif
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
            bool success = reserve(cap + increase_by + ((32 + sizeof(value_type)) / sizeof(value_type)));

#ifdef DEBUG
            // TODO: Do a debug log print here to notify of allocation failure
#endif
        }
    }


    void raw_insert(value_type* a, value_type* to_insert_pos, const value_type* to_insert_value)
    {
        // NOTE: may not be very efficient (underlying allocator may need to realloc/copy etc.
        // so later consider doing the insert operation at that level)
        ensure_additional_capacity(1);

        helper.size(helper.size() + 1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = to_insert_pos - a;
        size_type remaining = size() - raw_typed_pos;

        // FIX: This is causing a memory allocation issue, probably a buffer overrun
        // but not sure why
        memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));

        *to_insert_pos = *to_insert_value;
    }

public:
    dynamic_array() {}

    template <class THelperParam>
    dynamic_array(THelperParam& p) :
            helper(p) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    dynamic_array(std::initializer_list<value_type> initlist)
    {
        pointer p = lock();

        std::copy(initlist.begin(), initlist.end(), p);

        unlock();
    }
#endif

    // TODO: iterate through and destruct elements
    ~dynamic_array() {}

    allocator_type& get_allocator()
    {
        return helper.get_allocator();
    }

    size_type size() const { return helper.size(); }

    size_type capacity() const
    {
        return helper.capacity();
    }

    // we deviate from spec because we don't use exceptions, so a manual check for reserve failure is required
    // return true = successful reserve, false = fail
    bool reserve( size_type new_cap )
    {
        if(!helper.is_allocated())
            return helper.allocate(new_cap);
        else
            return helper.reallocate(new_cap);
    }

protected:
    void _append(const value_type* buf, size_type len)
    {
        ensure_additional_capacity(len);

        // Doing this before memcpy for null-terminated
        // scenarios
        size_type current_size = size();

        helper.size(current_size + len);

        value_type* raw = lock(current_size);

        while(len--) *raw++ = *buf++;

        unlock();
    }

    // internal version of replace not conforming to standard
    // (standard version also inserts or removes characters if requested,
    //  this one ONLY replaces the entire buffer)
    // TODO: change to assign
    void assign(const value_type* buf, size_type len)
    {
        reserve(len);

        helper.size(len);

        value_type* raw = lock();

        while(len--) *raw++ = *buf++;

        unlock();
    }

    // basically raw_erase and maps almost directly to string::erase with numeric index
    // will need a bit of wrapping to interact with iterators
    void _erase(size_type index, size_type count)
    {
        pointer raw = lock(index);

        // TODO: optimize null-terminated flavor to not use memmove at all
        size_type prev_size = helper.size();

        if(helper_type::uses_termination())
            // null terminated flavor merely includes null termination as part
            // of move
            prev_size++;
        else
            helper.size(prev_size - count);

        memmove(raw, raw + count, prev_size - (index + count));

        unlock();
    }

public:
    void pop_back()
    {
        // decrement the end of the array
        size_type end = helper.size() - 1;

        // lock down element at that position and run the destructor
        helper.lock(end).~value_type();
        helper.unlock();

        // TODO: put in warning if this doesn't work, remember
        // documentation says 'undefined' behavior if empty
        // so nothing to worry about too much
        helper.size(end);
    }

    void push_back(const value_type& value)
    {
        _append(&value, 1);
    }

    template <class ForeignAllocator, class ForeignHelper>
    dynamic_array& operator=(const dynamic_array<ForeignAllocator, ForeignHelper>& copy_from)
    {
        reserve(copy_from.size());
        copy_from.copy(lock(), capacity());
        unlock();
        return *this;
    }

    //typedef estd::internal::accessor<TAllocator> accessor;

    // TODO: make accessor do this comparison in a self contained way
    typedef typename std::conditional<allocator_traits::is_stateful(),
                accessor<allocator_type>,
                accessor_stateless<allocator_type> >::type
                accessor;

    class iterator
    {
    private:
        accessor current;

    public:
        // All-or-nothing, though not supposed to be that way till C++17 but is sometimes
        // before that (http://en.cppreference.com/w/cpp/iterator/iterator_traits)
        typedef dynamic_array::value_type value_type;
        typedef int difference_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef ::std::forward_iterator_tag iterator_category;

        value_type& lock() { return current.lock(); }
        void unlock() { current.unlock(); }


        iterator(allocator_type& allocator, const handle_with_offset& h ) :
            current(allocator, h)
        {

        }

        iterator(const iterator& copy_from) : current(copy_from.current) {}

        ~iterator()
        {
            // FIX: not so great.  It might be passable, though not recommended,
            // to unlock an alread unlocked value.  But it's less certain what
            // happens when we only unlock once if many lock() calls have happened
            // -- another reason to consolidate with iterators/list, because it
            // attempts to deal with this a little bit with a lock counter
            unlock();
        }

        // prefix version
        iterator& operator++()
        {
            current.h_exp().increment();
            return *this;
        }


        iterator& operator--()
        {
            current.h_exp().increment(-1);
            return *this;
        }

        // postfix version
        iterator operator++(int)
        {
            iterator temp(*this);
            operator++();
            return temp;
        }

        // postfix version
        iterator operator--(int)
        {
            iterator temp(*this);
            operator--();
            return temp;
        }

        bool operator==(const iterator& compare_to) const
        {
            return current.h_exp() == compare_to.current.h_exp();
        }

        bool operator!=(const iterator& compare_to) const
        {
            return !(operator ==)(compare_to);
            //return current != compare_to.current;
        }

        value_type& operator*()
        {
            // TODO: consolidate with InputIterator behavior from iterators/list.h
            return lock();
        }

        const value_type& operator*() const
        {
            return lock();
        }
    };

    typedef const iterator const_iterator;


    iterator begin()
    {
        return iterator(get_allocator(), offset(0));
    }

    iterator end()
    {
        handle_with_offset o = offset(size());

        return iterator(get_allocator(), o);
    }


    const_iterator end() const
    {
        handle_with_offset o = offset(size());

        return iterator(get_allocator(), o);
    }


    accessor operator[](size_type pos)
    {
        return accessor(get_allocator(), offset(pos));
    }

    accessor front()
    {
        return accessor(get_allocator(), offset(0));
    }

    accessor back()
    {
        return accessor(get_allocator(), offset(size() - 1));
    }

    accessor at(size_type pos)
    {
        // TODO: place in error/bounds checking runtime ability within
        // accessor itself.  perhaps wrap it all up in a FEATURE_ESTD_BOUNDSCHECK
        // to compensate for lack of exceptions requiring additional
        // data in accessor itself
        return accessor(get_allocator(), offset(pos));
    }
};


}

}
