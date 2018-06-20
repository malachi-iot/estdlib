#pragma once

#include "../memory.h"
#include "runtime_array.h"

#ifdef FEATURE_CPP_INITIALIZER_LIST
#include <initializer_list>
#include <algorithm>
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
#include <utility> // for std::forward
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
public:
    // default implementation is 'full fat' to handle all scenarios
    typedef TAllocator allocator_type;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

private:
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

    handle_with_offset offset(size_type pos) const
    {
        return allocator.offset(handle, pos);
    }

    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return allocator_traits::lock(allocator, handle, pos, count);
    }

    // constant-return-lock
    const value_type& clock(size_type pos = 0, size_type count = 0) const
    {
        return allocator.clock(handle, pos, count);
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

    bool empty() const { return m_size == 0; }
};


// non standard base class for managing expanding/contracting arrays
// accounts for lock/unlock behaviors. Used for vector and string
// More or less 1:1 with vector
// and may get rolled back completely into vector at some point -
// size_tracker_* are very experimental
template <class TAllocator, class THelper = dynamic_array_helper<TAllocator > >
class dynamic_array : public allocated_array<THelper>
{
    typedef dynamic_array this_t;
    typedef allocated_array<THelper> base_t;

public:
    typedef TAllocator allocator_type;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef THelper helper_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::pointer pointer;
    //typedef typename allocator_traits::reference reference; // one of our allocator_traits doesn't reveal this but I can't figure out which one
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef typename base_t::accessor accessor;

    // TODO: utilize SFINAE if we can
    // ala https://stackoverflow.com/questions/7834226/detecting-typedef-at-compile-time-template-metaprogramming
    //typedef typename allocator_type::accessor accessor_experimental;

protected:
    helper_type& helper() { return base_t::m_helper; }

    const helper_type& helper() const { return base_t::m_helper; }

#ifdef FEATURE_ESTD_LOCK_COUNTER
    typename allocator_traits::lock_counter lock_counter;
#endif

public:
    // Always try to avoid explicit locking and unlocking ... but sometimes
    // you gotta do it, so these are public
    value_type* lock(size_type pos = 0, size_type count = 0)
    {
#ifdef FEATURE_ESTD_LOCK_COUNTER
        lock_counter++;
#endif
        return &helper().lock(pos, count);
    }

    void unlock()
    {
#ifdef FEATURE_ESTD_LOCK_COUNTER
        lock_counter--;
#endif
        helper().unlock();
    }

protected:
    const value_type* clock_experimental(size_type pos = 0, size_type count = 0)
    {
#ifdef FEATURE_ESTD_LOCK_COUNTER
        lock_counter++;
#endif
        return &helper().clock(pos, count);
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


    // internal method for reassigning size, ensuring capacity is available
    void ensure_total_size(size_type new_size, size_type pad = 0, bool shrink = false)
    {
        size_type cap = capacity();

        if(new_size > cap)
        {
            // TODO: Do an assert here, or return true/false to indicate success
            reserve(new_size + pad);
        }

        helper().size(new_size);

        if(shrink) shrink_to_fit();
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
    void raw_insert(value_type* a, value_type* to_insert_pos, value_type&& to_insert_value)
    {
        grow(1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = to_insert_pos - a;
        size_type remaining = size() - raw_typed_pos;

        // FIX: This is causing a memory allocation issue, probably a buffer overrun
        // but not sure why
        memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));

        new (to_insert_pos) value_type(std::move(to_insert_value));
    }
#endif


    void raw_insert(value_type* a, value_type* to_insert_pos, const value_type* to_insert_value)
    {
        // NOTE: may not be very efficient (underlying allocator may need to realloc/copy etc.
        // so later consider doing the insert operation at that level)
        ensure_additional_capacity(1);

        helper().size(helper().size() + 1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = to_insert_pos - a;
        size_type remaining = size() - raw_typed_pos;

        // FIX: This is causing a memory allocation issue, probably a buffer overrun
        // but not sure why
        memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));

        *to_insert_pos = *to_insert_value;
    }

    dynamic_array(const dynamic_array& copy_from) :
        // FIX: Kinda ugly, we do this to force going thru base class'
        // copy constructor rather than the THelperParam flavor
        base_t(static_cast<const base_t&>(copy_from))
    {
    }

public:
    dynamic_array() {}

    template <class THelperParam>
    dynamic_array(const THelperParam& p) :
            base_t(p) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    dynamic_array(std::initializer_list<value_type> initlist)
    {
        pointer p = lock();

        std::copy(initlist.begin(), initlist.end(), p);

        helper().size(initlist.size());

        unlock();
    }
#endif

    // TODO: iterate through and destruct elements
    ~dynamic_array() {}

    // FIX: First of all , don't like forcing const
    // secondly, exposing these as public is not great - but eventually
    // we might be forced to expose some form of lock/unlock as public
    // currently only exposing as public to make it easier for compare() to work
    const value_type* fake_const_lock(size_type pos = 0, size_type count = 0) const
    {
        // Ugly, and would use decltype if I wasn't concerned with pre C++11 compat
        return const_cast<this_t*>(this)->clock_experimental(pos, count);
    }

    void fake_const_unlock() const
    {
        return const_cast<this_t*>(this)->unlock();
    }

    size_type size() const { return base_t::size(); }

    size_type capacity() const
    {
        return helper().capacity();
    }

    // we deviate from spec because we don't use exceptions, so a manual check for reserve failure is required
    // return true = successful reserve, false = fail
    bool reserve( size_type new_cap )
    {
        if(!helper().is_allocated())
            return helper().allocate(new_cap);
        else
            return helper().reallocate(new_cap);
    }

protected:
    // returns size before growth
    // internal call
    size_type grow(int by_amount)
    {
        ensure_additional_capacity(by_amount);

        // Doing this before memcpy for null-terminated
        // scenarios
        size_type current_size = size();

        helper().size(current_size + by_amount);

        return current_size;
    }


    void _append(const value_type* buf, size_type len)
    {
        size_type current_size = grow(len);

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
        ensure_total_size(len);

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
        size_type prev_size = helper().size();

        if(helper_type::uses_termination())
            // null terminated flavor merely includes null termination as part
            // of move
            prev_size++;
        else
            helper().size(prev_size - count);

        memmove(raw, raw + count, prev_size - (index + count));

        unlock();
    }


    /* a bit of a wrinkle, string uses traits_type to compare here but we do not
    template <class TForeignHelper>
    int compare(const dynamic_array<typename ForeignHelper::allocator_type, ForeignHelper>& compare_to) const
    {
        size_type raw_size = size();
        size_type s_size = compare_to.size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const CharT* raw = fake_const_lock();
        const CharT* s = compare_to.fake_const_lock();

        int result = traits_type::compare(raw, s, raw_size);

        fake_const_unlock();

        str.fake_const_unlock();

        return result;
    } */



    template <class ForeignHelper>
    bool starts_with(const dynamic_array<typename ForeignHelper::allocator_type, ForeignHelper>& compare_to) const
    {
        const value_type* s = fake_const_lock();
        const value_type* t = compare_to.fake_const_lock();

        size_type source_max = size();
        size_type target_max = compare_to.size();

        while(source_max-- && target_max--)
            if(*s++ != *t++)
            {
                fake_const_unlock();
                return false;
            }

        fake_const_unlock();
        // if compare_to is longer than we are, then it's also a fail
        return source_max != -1;
    }


public:
    // copy (into dest)
    // officially only for basic_string, but lives happily here in dynamic_array
    size_type copy(typename estd::remove_const<value_type>::type* dest,
                   size_type count, size_type pos = 0) const
    {
        const value_type* src = fake_const_lock();

        // TODO: since we aren't gonna throw an exception, determine what to do if
        // pos > size()

        if(pos + count > size())
            count = size() - pos;

        memcpy(dest, src + pos, count * sizeof(value_type));

        fake_const_unlock();

        return count;
    }


    bool empty() const
    {
        return helper().empty();
    }

    template <class TForeignAllocator, class TDAHelper>
    dynamic_array& append(const dynamic_array<TForeignAllocator, TDAHelper>& str)
    {
        size_type len = str.size();

        const typename TForeignAllocator::value_type* append_from = str.fake_const_lock();

        _append(append_from, len);

        str.fake_const_unlock();

        return *this;
    }


    void pop_back()
    {
        // decrement the end of the array
        size_type end = helper().size() - 1;

        // lock down element at that position and run the destructor
        helper().lock(end).~value_type();
        helper().unlock();

        // TODO: put in warning if this doesn't work, remember
        // documentation says 'undefined' behavior if empty
        // so nothing to worry about too much
        helper().size(end);
    }

    void push_back(const value_type& value)
    {
        _append(&value, 1);
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    void push_back(value_type&& value)
    {
        // TODO: combine this with _append since it's mostly overlapping code
        size_type current_size = grow(1);

        value_type* raw = lock(current_size);

        new (raw) value_type(std::move(value));

        unlock();
    }
#endif

    template <class ForeignHelper>
    dynamic_array& operator=(const dynamic_array<typename ForeignHelper::allocator_type, ForeignHelper>& copy_from)
    {
        ensure_total_size(copy_from.size());
        copy_from.copy(lock(), capacity());
        unlock();
        return *this;
    }

    //typedef estd::internal::accessor<TAllocator> accessor;





    typedef const iterator const_iterator;


    // untested and unoptimized
    iterator erase(const_iterator pos)
    {
        size_type index = pos - base_t::begin();
        _erase(index, 1);
        // chances are iterator is a copy of incoming pos,
        // but we'll do this anyway
        return iterator(base_t::get_allocator(), base_t::offset(index));
    }




#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    accessor emplace_back(TArgs&&...args)
    {
        // TODO: combine this with _append since it's mostly overlapping code
        size_type current_size = grow(1);

        value_type* raw = lock(current_size);

        allocator_traits::construct(base_t::get_allocator(), raw, std::forward<TArgs>(args)...);

        unlock();

        return base_t::back();
    }
#endif

    // NOTE: because pos requires a non-const lock, we can't do traditional
    // const_iterator here
    iterator insert(iterator pos, const_reference value)
    {
        pointer a = lock();

        reference pos_item = pos.lock();

        // all very raw array dependent
        raw_insert(a, &pos_item, &value);

        pos.unlock();

        unlock();

        return pos;
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    iterator insert(iterator pos, value_type&& value)
    {
        pointer a = lock();

        reference pos_item = pos.lock();

        // all very raw array dependent
        raw_insert(a, &pos_item, std::move(value));

        pos.unlock();

        unlock();

        return pos;
    }
#endif

    void shrink_to_fit()
    {
        reserve(size());
    }
};


}

}
