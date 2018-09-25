#pragma once

#include "allocated_array.h"
#include "../../allocators/fixed.h"
#include "../../allocators/handle_desc.h"

namespace estd { namespace internal { namespace impl {


// See reference implementation near the bottom
template <class TAllocator, class TPolicy>
struct dynamic_array;

// TODO: Fixup name.  Specializer to reveal size of either
// an explicitly-sized or null-terminated entity
template <class TAllocator, bool null_terminated, bool size_equals_capacity>
struct dynamic_array_length;

// null terminated
template <class TAllocator>
struct dynamic_array_length<TAllocator, true, false>
{
    typedef estd::handle_descriptor<TAllocator> handle_descriptor;
    typedef typename estd::remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::handle_type handle_type;

    bool empty(const allocator_type& a, const handle_type& h) const
    {
        const value_type* v = &a.clock(h, 0, 1);

        bool is_terminator = *v == 0;

        a.cunlock(h);

        return is_terminator;
    }


    bool empty(const handle_descriptor& hd) const
    {
        bool is_terminator = hd.clock(0, 1) == 0;
        hd.cunlock();
        return is_terminator;
    }

    size_type size(const TAllocator& a, const handle_type& h) const
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        // specialization required if we aren't null terminated (to track size variable)
        //static_assert(null_terminated, "Utilizing this size method requires null termination = true");
#endif

        const value_type* s = &a.clock(h);

        // FIX: use char_traits string length instead
        size_type sz = strlen(s);

        a.cunlock(h);

        return sz;
    }

    size_type size(const handle_descriptor& hd) const
    {
        const value_type* s = &hd.clock();

        size_type sz = strlen(s);

        hd.cunlock();

        return sz;
    }

    // +++ temporary
    // semi-brute forces size by stuffing a null terminator at the specified spot
    void size(TAllocator& a, const handle_type& h, size_type len)
    {
        /*
        if(len > base_t::capacity())
        {
            // FIX: issue some kind of warning
        } */

        a.lock(h, len, 1) = 0;
        a.unlock(h);
    }

    void size(handle_descriptor& hd, size_type len)
    {
        /*
        if(len > base_t::capacity())
        {
            // FIX: issue some kind of warning
        } */

        hd.lock(len,1) = 0;
        hd.unlock();
    }
    // ---
};


// explicitly sized
template <class TAllocator>
struct dynamic_array_length<TAllocator, false, false>
{
private:
    typedef typename estd::remove_reference<TAllocator>::type allocator_type;
    typedef typename estd::allocator_traits<allocator_type> allocator_traits;
    typedef estd::handle_descriptor<TAllocator> handle_descriptor;

public:
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_traits::handle_type handle_type;

protected:
    size_type m_size;

    dynamic_array_length() :
            m_size(0)
    {}


public:

    bool empty(const handle_descriptor& hd) const
    {
        return m_size == 0;
    }

    bool empty(const allocator_type& a, const handle_type& h) const
    {
        return m_size == 0;
    }

    size_type size(const TAllocator& a, const handle_type& h) const
    {
        return m_size;
    }

    size_type size(const handle_descriptor& hd) const
    {
        return m_size;
    }


    // +++ temporary
    void size(TAllocator& a, const handle_type& h, size_type len)
    {
        m_size = len;
    }

    void size(handle_descriptor& hd, size_type len)
    {
        m_size = len;
    }
    // ---
};

// ---

// non-sized, this specialization is for when dynamic_array size is
// always the same as capacity() (which is represented by handle_descriptor::size())
// NOTE: This is null-terminated variety, which does extra evaluation of underlying
//       data compared to our fixed-length buffer size.  May end up being identical to
//       existing null-terminated dynamic_array_length handler
// NOTE: Not yet used or tested
template <class TAllocator>
struct dynamic_array_length<TAllocator, true, true>
{
private:
    typedef typename estd::remove_reference<TAllocator>::type allocator_type;
    typedef typename estd::allocator_traits<allocator_type> allocator_traits;
    typedef estd::handle_descriptor<TAllocator> handle_descriptor;

public:
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;

    bool empty(const handle_descriptor& hd) const
    {
        // FIX: still resolving what size resolves to be for layer2 unbounded null terminated
        // strings
        bool empty = hd.clock(0, 1) == 0;

        hd.cunlock();

        return empty;
        //return hd.size() > 0;
    }

    size_type size(const handle_descriptor& hd) const
    {
        const value_type* s = &hd.clock();
        const size_type max_size = hd.size();

        size_type sz = strlen(s);

        hd.cunlock();

        if(max_size != 0 && sz >= max_size)
        {
            // TODO: null terminated should always be less than allocated size, so this
            // indicates a runtime-discovered error
        }

        return sz;
        //return hd.size();
    }
};


// non-sized, this specialization is for when dynamic_array size is
// always the same as capacity() (which is represented by handle_descriptor::size())
template <class TAllocator>
struct dynamic_array_length<TAllocator, false, true>
{
private:
    typedef typename estd::remove_reference<TAllocator>::type allocator_type;
    typedef typename estd::allocator_traits<allocator_type> allocator_traits;
    typedef estd::handle_descriptor<TAllocator> handle_descriptor;

public:
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;

    bool empty(const handle_descriptor& hd) const
    {
        return hd.size() > 0;
    }

    size_type size(const handle_descriptor& hd) const
    {
        return hd.size();
    }
};


// intermediate class as we transition to handle_descriptor.  Its primary purpose now
// is to present sizing/capacity operations to consumer.
// null_terminated flag takes on no meaning when size_equals_capacity is true
//
// size_equals_capacity flag is just that, means the dynamic_array::size() is more or less a constant
// fed by wherever capacity() comes from.  capacity() will vary on whether allocator itself tracks
// allocated size or not (underlying size() call)
template <class TAllocator, bool null_terminated, bool size_equals_capacity>
class dynamic_array_base :
        public estd::internal::impl::allocated_array<TAllocator>,
        dynamic_array_length<TAllocator, null_terminated, size_equals_capacity>
{
    typedef estd::internal::impl::allocated_array<TAllocator> base_t;
    typedef dynamic_array_length<TAllocator, null_terminated, size_equals_capacity> length_helper_t;

public:
    static CONSTEXPR bool uses_termination() { return null_terminated; }

    // this works better with < c++11
    static CONSTEXPR bool uses_termination_exp = null_terminated;

    typedef typename base_t::allocator_type allocator_type;
    // NOTE: Necessary to do allocator_traits here to disambiguate all the ones from
    // multiple inheritance above (though I think that should have worked, unless
    // they are resolving to different types... i.e. one is a TAllocator&)
    typedef typename base_t::allocator_traits allocator_traits;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    //typedef typename allocator_traits::handle_with_offset handle_with_offset;

    // account for null-termination during a max_size request
    size_type max_size() const
    {
        return base_t::max_size() - (null_terminated ? 1 : 0);
        //return base_t::get_allocator().max_size() - (null_terminated ? 1 : 0);
    }


    // repurposing/renaming of what size meant before (ALLOCATED) vs now
    // (USED within ALLOCATED)
    size_type capacity() const { return base_t::size(); }

    /*
    // Helper for old dynamic_array code.  New one I'm thinking caller
    // can shoulder this burden
    handle_with_offset offset(size_type pos) const
    {
        return base_t::get_allocator().offset(base_t::handle(), pos);
    } */

    // remember, dynamic_array_helper size() refers not to ALLOCATED size, but rather
    // 'used' size within that allocation.  For this variety, we are null terminated
    size_type size() const
    {
        if(!base_t::is_allocated()) return 0;

        return length_helper_t::size(*this);
        //return length_helper_t::size(base_t::get_allocator(), base_t::handle());
    }

    void size(size_type n)
    {
        length_helper_t::size(*this, n);
        //length_helper_t::size(base_t::get_allocator(), base_t::handle(), n);
    }

    bool empty() const
    {
        return length_helper_t::empty(*this);
        //return length_helper_t::empty(base_t::get_allocator(), base_t::handle());
    }

    dynamic_array_base()
    {
        // FIX: A little sloppy, brute forcing to 0 here because null-terminated
        // specialization doesn't have all the data it needs to do this
        // Sloppy also because for explicitly sized scenario, it already initialies to 0
        size(0);
    }

    template <class TAllocatorParameter>
    dynamic_array_base(TAllocatorParameter& p) : base_t(p)
    {
        // FIX: More sloppy, it's possible TAllocatorParameter has the sizing data in it
        // so we do NOT do size here
        //size(0);
    }


};

/*
// applies generally to T[N], RW buffer but also to non-const T*
// applies specifically to null-terminated
template <class T, size_t len, class TBuffer, class TPolicy>
class dynamic_array<single_fixedbuf_allocator<T, len, true, TBuffer>, TPolicy>
        : public dynamic_array_base<single_fixedbuf_allocator<T, len, true, TBuffer>, true >
{
    typedef dynamic_array_base<single_fixedbuf_allocator<T, len, true, TBuffer>, true > base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::handle_type handle_type;

    // FIX: Iron out exactly where we really assign size(0) - sometimes we
    // want to pre-initialize our buffer so size(0) is not an always thing
    dynamic_array(TBuffer& b) : base_t(b)
    {
        base_t::size(0);
    }

    dynamic_array(const TBuffer& b) : base_t(b)
    {
        // FIX: This only works for NULL-terminated scenarios
        // we still need to assign a length of 0 for explicit lenght scenarios
    }

    dynamic_array()
    {
        base_t::size(0);
    }
}; */





#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
// runtime (layer3-ish) version, no policy (void) -
// hard wired to no null termination
// currently *also* hard wired to size != capacity, but we'll likely want that to change for const_string
template <class T>
class dynamic_array<single_fixedbuf_runtimesize_allocator<T>, void> :
        public dynamic_array_base<single_fixedbuf_runtimesize_allocator<T>, false, false >
{
    typedef dynamic_array_base<single_fixedbuf_runtimesize_allocator<T>, false, false > base_t;
    typedef typename base_t::size_type size_type;

public:
    template <class TInitParam>
    dynamic_array(const TInitParam& p) : base_t(p) {}
};
#endif


#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
// experimental feature, has_typedef (lifted from PGGCC-13)
template <typename>
struct has_typedef { typedef void type; };

template<typename T, typename = void>
struct is_nulltag_present : estd::false_type {};

template<typename T>
struct is_nulltag_present<T, typename has_typedef<typename T::is_null_terminated_exp_tag>::type> : estd::true_type {};

template<typename T, typename = void>
struct is_consttag_present : estd::false_type {};

template<typename T>
struct is_consttag_present<T, typename has_typedef<typename T::is_constant_tag_exp>::type> : estd::true_type {};

// FIX: Hard wired to null terminated !
template <class TAllocator, class TPolicy>
struct dynamic_array : public
        dynamic_array_base<
            typename estd::remove_reference<TAllocator>::type,
            is_nulltag_present<TPolicy>::value,
            is_consttag_present<TPolicy>::value>
{
    typedef dynamic_array_base<
        typename estd::remove_reference<TAllocator>::type,
        is_nulltag_present<TPolicy>::value,
        is_consttag_present<TPolicy>::value> base_t;
    typedef typename base_t::allocator_type allocator_type;

    dynamic_array(allocator_type& alloc) : base_t(alloc) {}

    template <class TAllocatorParam>
    dynamic_array(const TAllocatorParam& p) : base_t(p) {}

    dynamic_array() {}
};
#else
// General-case dynamic_array where we don't attempt to optimize anything.  This is a fullback
// TODO: #ifdef this out in some kind of strict mode
template <class TAllocator, class TPolicy>
class dynamic_array
{
public:
    // default implementation is 'full fat' to handle all scenarios
    typedef typename estd::remove_reference<TAllocator>::type allocator_type;
    typedef estd::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

private:
    // don't fiddle with ref juggling here - if that's absolutely necessary use
    // the RefAllocator helper
    TAllocator allocator;
    // handle.size represents currently allocation portion
    handle_with_size handle;
    // remember, size represents 'user/app' portion.
    size_type m_size;

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

    void cunlock() const
    {
        allocator_traits::cunlock(allocator, handle);
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

    explicit dynamic_array(allocator_type& alloc) :
        allocator(alloc),
        handle(allocator_traits::invalid()),
        m_size(0)
    {

    }

    template <class T>
    dynamic_array(T init) :
            allocator(init),
            handle(allocator_traits::invalid()),
            m_size(0)
    {

    }

    dynamic_array() :
            handle(allocator_traits::invalid()),
            m_size(0)
    {

    }

    ~dynamic_array()
    {
        if(handle != allocator_traits::invalid())
            allocator.deallocate(handle, 1);
    }

    bool empty() const { return m_size == 0; }
};
#endif

}}}
