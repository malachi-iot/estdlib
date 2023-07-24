#pragma once

#include "../memory.h"
#include "runtime_array.h"
#include "impl/dynamic_array.h"

#include "../initializer_list.h"
#include "../utility.h"
#include "../new.h"

#include "../expected.h"
#include "feature/dynamic_array.h"

namespace estd {

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
    typedef allocated_array<TImpl> base_type;

public:
    typedef typename base_type::allocator_type allocator_type;
    typedef typename base_type::allocator_traits allocator_traits;
    typedef typename base_type::impl_type impl_type;
    typedef typename base_type::value_type value_type;

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
    // Use this instead of 'success' ptr
    // If success, grew by requested value - returns the size we started with
    // If in error, returns how much was actually grown
    //typedef estd::expected<size_type, size_type> grow_result;

    struct grow_result
    {
        const size_type starting_size;
        /// "increased by" in either case, but if in error state it's a reduced count than
        /// what was expected
        const estd::expected<size_type, size_type> increased_by;

        ESTD_CPP_CONSTEXPR_RET grow_result(size_type starting_size, size_type increased_by) :
            starting_size(starting_size),
            increased_by(increased_by)
        {}

        ESTD_CPP_CONSTEXPR_RET grow_result(unexpect_t, size_type starting_size, size_type increased_by) :
            starting_size(starting_size),
            increased_by(unexpect_t(), increased_by)
        {}
    };

    // internal method for auto increasing capacity based on pre-set amount
    grow_result ensure_additional_capacity(size_type increase_by)
    {
        // DEBT: fixed allocators IIRC have matching capacity() and max_size()
        // possibly resulting in big and crusty code here

        const size_type starting_size = size();
        const size_type cap = capacity();
        bool success = true;

        // TODO: assert increase_by is a sensible value
        // above 0 and less than ... something

        if(starting_size + increase_by > cap)
        {
            // increase by as near to 32 bytes as is practical
            // DEBT: Really need to do this by some kind of policy
            const size_type requested_size = cap + increase_by + ((32 + sizeof(value_type)) / sizeof(value_type));

            success = reserve(requested_size);

            /*
            // NOTE: fixed allocator aborts on reallocate call, and now we filter that out with
            // this max_size.  The idea is that dynamic_array avoids aborts/exceptions - but consuming
            // things like vector/string will actually be responsible for that.
            if(requested_size <= impl().max_size())
                success = reserve(requested_size);
            else
                // DEBT: Doing this because fixed allocators currently just call 'abort' on reallocate
                success = false;
            */

#ifdef DEBUG
            // TODO: Do a debug log print here to notify of allocation failure
#endif
        }

        if(success)
            return grow_result(starting_size, increase_by);
        else
            return grow_result(unexpect_t(), starting_size, size() - starting_size);
    }


    // internal method for reassigning size, ensuring capacity is available
    // DEBT: Probably need to use grow_result here too
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

    ESTD_CPP_CONSTEXPR_RET size_type size() const { return base_t::size(); }   // NOLINT

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
    /// @returns size before growth
    grow_result grow(int by_amount)
    {
        grow_result r = ensure_additional_capacity(by_amount);

        if(r.increased_by.has_value())
            impl().size(r.starting_size + by_amount);
        else
            impl().size(r.starting_size + r.increased_by.error());

        return r;
    }

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
    // If success, returns number of bytes appended which matches initial request
    // If error, returns number of bytes actually appended, which may be 0
    // TODO: Try to make this a layer1 kind of thing, to avoid extra bool
    typedef estd::expected<size_type, size_type> append_result;

    // Not wanting this because I'd like to gravitate towards always returning byte count written
    //typedef estd::layer1::optional<size_type, (size_type)-1> append_result;
#else
    typedef size_type append_result;
#endif


    // somewhat non standard, but considered a real API in estd
    // in theory this might go a little faster than the iterator
    // version
    append_result append(const value_type* buf, size_type len)
    {
        grow_result r = grow(len);
        const size_type current_size = r.starting_size;
#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        const bool grow_success = r.increased_by.has_value();

#if FEATURE_ESTD_DYNAMIC_ARRAY_APPEND_TRUNC
        // DEBT: Overflow not actually tested yet
        // DEBT: We'd prefer a version of grow which actually output new len, since size()
        // can be a little expensive
        if(grow_success == false)   len = r.error();
#else
        if(grow_success == false)   return append_result(unexpect_t(), 0);
#endif

#endif

        value_type* raw = lock(current_size);

        estd::copy_n(buf, len, raw);

        unlock();

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        if(grow_success)
            return append_result(len);
        else
            return append_result(unexpect_t(), len);
#else
        return len;
#endif
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
    // EXPERIMENTAL, lightly tested
    template <class TImpl2>
    append_result append(const experimental::private_array<TImpl2>& source)
    {
        //typedef typename experimental::private_array<TImpl2>::const_iterator iterator;
        size_type len = source.size();

        grow_result r = grow(len);
        const size_type pre_growth_size = r.starting_size;
#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        const bool grow_success = r.increased_by.has_value();

#if FEATURE_ESTD_DYNAMIC_ARRAY_APPEND_TRUNC
        if(grow_success == false)   len = r.increased_by.error();
#else
        if(grow_success == false)   return append_result(unexpect_t(), 0);
#endif
#endif

        pointer raw = lock(pre_growth_size);

        source.copy_ll(raw, len);
        //source.copy(raw, len);

        // NOTE: Not using source.copy because we manually did source.size()
        // already, which for expliclty sized would be a wash but for null-terminated
        // would incur an overhead since copy would call its own strlen

        //copy_n(source.begin(), len, raw);

        unlock();

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        return grow_success ?
            append_result(len) :
            append_result(estd::unexpect_t(), len);
#else
        return len;
#endif
    }

    template <class TForeignImpl>
    append_result append(const allocated_array<TForeignImpl>& source)
    {
        size_type len = source.size();

        const typename TForeignImpl::value_type* append_from = source.clock();

        append_result ar = append(append_from, len);

        source.cunlock();

        return ar;
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

    append_result push_back(const value_type& value)
    {
        return append(&value, 1);
    }

#ifdef __cpp_rvalue_references
    append_result push_back(value_type&& value)
    {
        // TODO: combine this with _append since it's mostly overlapping code
        grow_result r = grow(1);

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        if(r.increased_by.has_value() == false) return append_result(unexpect_t(), 0);
#endif

        const size_type current_size = r.starting_size;

        value_type* raw = lock(current_size);

        new (raw) value_type(std::move(value));

        unlock();

        return append_result();
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




#if __cpp_variadic_templates
    template <class ...TArgs>
    accessor emplace_back(TArgs&&...args)
    {
        // TODO: combine this with _append since it's mostly overlapping code
        grow_result r = grow(1);

        // FIX: Do bounds checking
        size_type current_size = r.starting_size;

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
