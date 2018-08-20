#pragma once

#include "../memory.h"
#include "runtime_array.h"
#include "impl/dynamic_array.h"

#include "../initializer_list.h"
#include "../utility.h"

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


// non standard base class for managing expanding/contracting arrays
// accounts for lock/unlock behaviors. Used for vector and string
// More or less 1:1 with vector
// and may get rolled back completely into vector at some point -
template <class TImpl>
class dynamic_array : public allocated_array<TImpl>
{
    typedef dynamic_array this_t;
    typedef allocated_array<TImpl> base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::allocator_traits allocator_traits;
    typedef typename base_t::impl_type impl_type;
    typedef typename base_t::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    //typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::pointer pointer;
    //typedef typename allocator_traits::reference reference; // one of our allocator_traits doesn't reveal this but I can't figure out which one
    typedef typename base_t::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef typename base_t::accessor accessor;

    // TODO: utilize SFINAE if we can
    // ala https://stackoverflow.com/questions/7834226/detecting-typedef-at-compile-time-template-metaprogramming
    //typedef typename allocator_type::accessor accessor_experimental;

protected:
    impl_type& impl() { return base_t::m_impl; }

    const impl_type& impl() const { return base_t::m_impl; }

public:
    // redeclared just for conveineince
    value_type* lock(size_type pos = 0, size_type count = 0)
    {
        return base_t::lock(pos, count);
    }

    void unlock() { return base_t::unlock(); }

protected:
    // internal method for auto increasing capacity based on pre-set amount
    bool ensure_additional_capacity(size_type increase_by)
    {
        size_type cap = capacity();
        bool success = true;

        // TODO: assert increase_by is a sensible value
        // above 0 and less than ... something

        if(size() + increase_by >= cap)
        {
            // increase by as near to 32 bytes as is practical
            success = reserve(cap + increase_by + ((32 + sizeof(value_type)) / sizeof(value_type)));

#ifdef DEBUG
            // TODO: Do a debug log print here to notify of allocation failure
#endif
        }

        return success;
    }


    // internal method for reassigning size, ensuring capacity is available
    bool ensure_total_size(size_type new_size, size_type pad = 0, bool shrink = false)
    {
        size_type cap = capacity();

        if(new_size > cap)
        {
            // TODO: Do an assert here, or return true/false to indicate success
            if(!reserve(new_size + pad)) return false;
        }

        impl().size(new_size);

        if(shrink) shrink_to_fit();

        return true;
    }


    // internal version of replace not conforming to standard
    // (standard version also inserts or removes characters if requested,
    //  this one ONLY replaces the entire buffer)
    // TODO: change to assign
    void assign(const value_type* buf, size_type len)
    {
        ensure_total_size(len);

        base_t::assign(buf, len);
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

        impl().size(impl().size() + 1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = to_insert_pos - a;
        size_type remaining = size() - raw_typed_pos;

        // FIX: This is causing a memory allocation issue, probably a buffer overrun
        // but not sure why
        memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));

        *to_insert_pos = *to_insert_value;
    }

    template <class TForeignImpl>
    dynamic_array(const allocated_array<TForeignImpl>& copy_from) :
            base_t(copy_from) {}

    /*
    dynamic_array(const dynamic_array& copy_from) :
        // FIX: Kinda ugly, we do this to force going thru base class'
        // copy constructor rather than the THelperParam flavor
        base_t(static_cast<const base_t&>(copy_from))
    {
    } */

public:
    dynamic_array() {}

    explicit dynamic_array(allocator_type& t) : base_t(t) {}

    template <class THelperParam>
    dynamic_array(const THelperParam& p) :
            base_t(p) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    dynamic_array(std::initializer_list<value_type> initlist)
        : base_t(initlist) {}
#endif

    // TODO: iterate through and destruct elements
    ~dynamic_array() {}

    size_type size() const { return base_t::size(); }

    size_type capacity() const { return impl().capacity(); }

    bool resize(size_type count)
    {
        return ensure_total_size(count);
    }

    void clear()
    {
        resize(0);
    }

    // we deviate from spec because we don't use exceptions, so a manual check for reserve failure is required
    // return true = successful reserve, false = fail
    bool reserve( size_type new_cap )
    {
        if(!impl().is_allocated())
            return impl().allocate(new_cap);
        else
            return impl().reallocate(new_cap);
    }

    template <class InputIt>
    void assign(InputIt first, InputIt last)
    {
        value_type* d = lock();

        ensure_total_size(last - first, 0, true);

        while(first != last)
            *d++ = *first++;

        unlock();
    }

protected:
    // internal call: grows entire size() by amount,
    // ensuring that there's enough space along the
    // way to do so (allocating more if necessary)
    // returns size before growth
    size_type grow(int by_amount)
    {
        ensure_additional_capacity(by_amount);

        // Doing this before memcpy for null-terminated
        // scenarios
        size_type current_size = size();

        impl().size(current_size + by_amount);

        return current_size;
    }


    // somewhat non standard, but considered a real API in estd
    // in theory this might go a little faster than the iterator
    // version
    dynamic_array& append(const value_type* buf, size_type len)
    {
        size_type current_size = grow(len);

        value_type* raw = lock(current_size);

        while(len--) *raw++ = *buf++;

        unlock();

        return *this;
    }

    // basically raw_erase and maps almost directly to string::erase with numeric index
    // will need a bit of wrapping to interact with iterators
    void _erase(size_type index, size_type count)
    {
        pointer raw = lock(index);

        // TODO: optimize null-terminated flavor to not use memmove at all
        size_type prev_size = impl().size();

        if(impl_type::uses_termination())
            // null terminated flavor merely includes null termination as part
            // of move
            prev_size++;
        else
            impl().size(prev_size - count);

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




public:
    template <class TForeignImpl>
    dynamic_array& append(const allocated_array<TForeignImpl>& source)
    {
        size_type len = source.size();

        const typename TForeignImpl::value_type* append_from = source.clock();

        append(append_from, len);

        source.cunlock();

        return *this;
    }


    void pop_back()
    {
        // decrement the end of the array
        size_type end = impl().size() - 1;

        // lock down element at that position and run the destructor
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        impl().destroy(end);
#else
        impl().lock(end).~value_type();
        impl().unlock();
#endif

        // TODO: put in warning if this doesn't work, remember
        // documentation says 'undefined' behavior if empty
        // so nothing to worry about too much
        impl().size(end);
    }

    void push_back(const value_type& value)
    {
        append(&value, 1);
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

    template <class TForeignImpl>
    dynamic_array& operator=(const allocated_array<TForeignImpl>& copy_from)
    {
        ensure_total_size(copy_from.size());
        copy_from.copy(lock(), capacity());
        unlock();
        return *this;
    }


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

#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        impl().construct(current_size, std::forward<TArgs>(args)...);
#else
        value_type* raw = lock(current_size);

        allocator_traits::construct(base_t::get_allocator(), raw, std::forward<TArgs>(args)...);

        unlock();
#endif

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
