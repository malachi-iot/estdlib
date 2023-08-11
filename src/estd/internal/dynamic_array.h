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

//
//  -
/// Base class for managing expanding/contracting arrays
/// Accounts for lock/unlock behaviors. Used for vector and string
/// @tparam TImpl typically estd::internal::impl::dynamic_array.  Abstracts away allocator-specific behaviors
/// @remarks Kind of a superset of vector.
/// EXPERIMENTAL: specializations via impl MIGHT be read/only i.e. not sizeable
template <class TImpl>
class dynamic_array : public allocated_array<TImpl>
{
    typedef allocated_array<TImpl> base_type;

protected:
    typedef dynamic_array_helper<TImpl> helper;

public:
    typedef typename base_type::allocator_type allocator_type;
    typedef typename base_type::allocator_traits allocator_traits;
    typedef typename base_type::impl_type impl_type;
    typedef typename base_type::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    //typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::pointer pointer;

    // DEBT: Getting errors due to ambiguity of source allocator_traits
    typedef const value_type* const_pointer;
    //typedef typename allocator_traits::const_pointer const_pointer;
    //typedef typename allocator_traits::reference reference; // one of our allocator_traits doesn't reveal this but I can't figure out which one
    typedef typename base_type::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_type::iterator iterator;
    typedef typename base_type::accessor accessor;

    // TODO: utilize SFINAE if we can
    // ala https://stackoverflow.com/questions/7834226/detecting-typedef-at-compile-time-template-metaprogramming
    //typedef typename allocator_type::accessor accessor_experimental;

protected:
    impl_type& impl() { return base_type::m_impl; }

    const impl_type& impl() const { return base_type::m_impl; }

public:
    // redeclared just for conveineince
    value_type* lock(size_type pos = 0, size_type count = 0)
    {
        return base_type::lock(pos, count);
    }

    void unlock() { return base_type::unlock(); }   // NOLINT

protected:
    // Use this instead of 'success' ptr
    // If success, grew by requested value - returns the size we started with
    // If in error, returns how much was actually grown
    //typedef estd::expected<size_type, size_type> grow_result;

    struct grow_result
    {
        const size_type starting_size;
#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
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
#else
        ESTD_CPP_CONSTEXPR_RET grow_result(size_type v) : starting_size(v) {}
#endif
    };

    // The following 3 ensure_xxx functions only pertain to:
    // - traditional dynamic memory scenarios (i.e. classic malloc/realloc/free behaviors)
    // - advanced and not yet implemented fragmented locking dynamic memory

    bool ensure_total_capacity(size_type new_size, size_type pad = 0)
    {
        if(new_size > capacity())
        {
            // TODO: Do an assert here, or return true/false to indicate success
            if(!reserve(new_size + pad)) return false;
        }

        return true;
    }

    // internal method for reassigning size, ensuring capacity is available
    bool ensure_total_size(size_type new_size, size_type pad = 0, bool shrink = false)
    {
        if(ensure_total_capacity(new_size, pad) == false)
            return false;

        impl().size(new_size);

        if(shrink) shrink_to_fit();

        return true;
    }

    // internal method for auto increasing capacity based on pre-set amount
    grow_result ensure_additional_capacity(size_type increase_by)
    {
        // DEBT: fixed allocators IIRC have matching capacity() and max_size()
        // possibly resulting in big and crusty code here

        const size_type starting_size = size();
        static constexpr size_type pad = ((32 + sizeof(value_type)) / sizeof(value_type));

        // TODO: assert increase_by is a sensible value
        // above 0 and less than ... something

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        const bool success = ensure_total_capacity(starting_size + increase_by, pad);

        if(success)
            return grow_result(starting_size, increase_by);
        else
            return grow_result(unexpect_t(), starting_size, size() - starting_size);
#else
        ensure_total_capacity(starting_size + increase_by, pad);
        return starting_size;
#endif
    }


    // internal version of replace not conforming to standard
    // (standard version also inserts or removes characters if requested,
    //  this one ONLY replaces the entire buffer)
    // TODO: change to assign
    void assign(const value_type* buf, size_type len)   // NOLINT
    {
        ensure_total_size(len);

        base_type::assign(buf, len);
    }

    // DEBT: Use a strictness feature flag to disallow falling back
    // to constructors
#ifdef __cpp_rvalue_references
    template <class T2 = value_type>
    typename enable_if<is_move_assignable<T2>::value>::type
    move_assist()
    {

    }

    template <class T2 = value_type>
    typename enable_if<is_nothrow_move_constructible<T2>::value>::type
    move_assist()
    {

    }
#endif

    template <class T2 = value_type>
    typename enable_if<is_copy_assignable<T2>::value>::type
    move_assist()
    {

    }



#ifdef __cpp_rvalue_references
    void raw_insert(value_type* a, value_type* to_insert_pos, value_type&& to_insert_value)
    {
        grow(1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        const size_type sz = size();
        size_type raw_typed_pos = to_insert_pos - a;
        size_type remaining = size() - raw_typed_pos;

        //*(to_insert_pos + 1) = std::move(*to_insert_pos);
        //new (to_insert_pos + 1) value_type(std::move(*to_insert_pos));

        estd::move_backward(to_insert_pos, (a + sz - 1), (a + sz));

        // FIX: This is causing a memory allocation issue, probably a buffer overrun
        // but not sure why
        //memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));

        new (to_insert_pos) value_type(std::move(to_insert_value));
    }
#endif


    void raw_insert(value_type* a, value_type* to_insert_pos, const value_type* to_insert_value)
    {
        // NOTE: may not be very efficient (underlying allocator may need to realloc/copy etc.
        // so later consider doing the insert operation at that level)
        const size_type sz = impl().size();
        ensure_total_size(sz + 1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = to_insert_pos - a;
        size_type remaining = sz - raw_typed_pos;

        // FIX: This is causing a memory allocation issue, probably a buffer overrun
        // but not sure why
        //memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));
        estd::copy_backward(to_insert_pos, (a + sz), (a + sz + 1));

        *to_insert_pos = *to_insert_value;
    }

    template <class TForeignImpl>
    ESTD_CPP_CONSTEXPR_RET EXPLICIT dynamic_array(
        const allocated_array<TForeignImpl>& copy_from) :
        base_type(copy_from) {}

    /*
    dynamic_array(const dynamic_array& copy_from) :
        // FIX: Kinda ugly, we do this to force going thru base class'
        // copy constructor rather than the THelperParam flavor
        base_t(static_cast<const base_t&>(copy_from))
    {
    } */

public:
    ESTD_CPP_DEFAULT_CTOR(dynamic_array)

    ESTD_CPP_CONSTEXPR_RET EXPLICIT dynamic_array(allocator_type& t) :
        base_type(t) {}

    // DEBT: a handle related compilation glitch occurs if we try to do perfect forwarding here
    //ESTD_CPP_FORWARDING_CTOR(dynamic_array)
    template <class Param1>
    ESTD_CPP_CONSTEXPR_RET EXPLICIT dynamic_array(const Param1 p1) :
        base_type(p1) {}

#if __cpp_initializer_lists
    constexpr dynamic_array(std::initializer_list<value_type> initlist)
        : base_type(initlist) {}
#endif

    // TODO: iterate through and destruct elements
    ~dynamic_array() {}

    ESTD_CPP_CONSTEXPR_RET size_type size() const   // NOLINT
    {
        return base_type::size();
    }

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
    // NOTE: This is kind of a lie, because reallocate will call abort() for fixed allocators
    bool reserve(size_type new_cap)
    {
        if(!impl().is_allocated())
            return impl().allocate(new_cap);
        else
            return impl().reallocate(new_cap);
    }

    template <class InputIt>
    void assign(InputIt first, InputIt last)
    {
        ensure_total_size(last - first, 0, true);

        helper::copy(*this, 0, first, last);
    }

//protected:
// DEBT: Temporarily making this public as we bring specializer online
public:
    // internal call: grows entire size() by amount,
    // ensuring that there's enough space along the
    // way to do so (allocating more if necessary)
    // only is relevant for true dynamic allocation scenarios, such as:
    // - classic malloc/realloc/free
    // - not-yet-implemented fragment-adapting locking memory
    /// @returns size before growth
    grow_result grow(size_type by_amount)
    {
        grow_result r = ensure_additional_capacity(by_amount);

        // No additional bounds checking, we rely on ensure_additional_capacity
        // for all that.  Would be nicer to do it in one fell swoop though, ACID-style

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        if(r.increased_by.has_value())
            impl().size(r.starting_size + by_amount);
        else
            impl().size(r.starting_size + r.increased_by.error());
#else
        impl().size(r.starting_size + by_amount);
#endif

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

        helper::copy_from(*this, current_size, buf, len);

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
    // EXPERIMENTAL, lightly tested - experimental::private array was a good try,
    // and inspired some ideas, and now is on the way out in favor of
    // allocator_locking_preference approach
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
        return helper::append(*this, source);
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

    template <class Impl2>
    void assign(const allocated_array<Impl2>& copy_from)
    {
        const unsigned len = copy_from.size();

        ensure_total_size(len);
        helper::copy_from(*this, 0, copy_from, len);
    }


    typedef const iterator const_iterator;


    // untested and unoptimized
    iterator erase(const_iterator pos)
    {
        size_type index = pos - base_type::begin();
        _erase(index, 1);
        // chances are iterator is a copy of incoming pos,
        // but we'll do this anyway
        return base_type::create_iterator(index);
        //return iterator(base_t::get_allocator(), base_t::offset(index));
    }




#if __cpp_variadic_templates
    template <class ...TArgs>
    accessor emplace_back(TArgs&&...args)
    {
        // TODO: combine this with _append since it's mostly overlapping code
        grow_result r = grow(1);

        // DEBT: Do bounds checking, though inherently we must do some within grow/ensure calls
        const size_type current_size = r.starting_size;

#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        impl().construct(current_size, std::forward<TArgs>(args)...);
#else
        value_type* raw = lock(current_size);

        allocator_traits::construct(base_t::get_allocator(), raw, std::forward<TArgs>(args)...);

        unlock();
#endif

        return base_type::back();
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

#if __cpp_rvalue_references
    iterator insert(iterator pos, value_type&& value)
    {
        pointer a = lock();

#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
        reference pos_item = *pos;
#else
        reference pos_item = pos.lock();
#endif

        // all very raw array dependent
        raw_insert(a, &pos_item, std::move(value));

#if !FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
        pos.unlock();
#endif

        unlock();

        return pos;
    }
#endif

    void shrink_to_fit()
    {
        reserve(size());
    }
};

// Special case dynamic array whose impl is NOT dynamic friendly.  Non specialized one
// works too, but it's nice to eliminate all that clutter for scenarios (string_view)
// which don't need it
template <class Allocator, class Policy>
struct dynamic_array<impl::allocated_array<Allocator, Policy> > :
    allocated_array<impl::allocated_array<Allocator, Policy> >
{
    typedef allocated_array<impl::allocated_array<Allocator, Policy> > base_type;

    ESTD_CPP_FORWARDING_CTOR(dynamic_array)
};

}

}
