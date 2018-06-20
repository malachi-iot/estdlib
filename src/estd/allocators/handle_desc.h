#pragma once

#include "../type_traits.h"
#include "../traits/allocator_traits.h"
#include <cassert>

namespace estd { namespace internal {

// TODO: Utilize allocator_traits throughout
// TODO: See if there's a smooth way to incorporate initial
//       offset in here as well or otherwise incorporate gracefully with handle_with_offset

// handle_descriptor provides rich data about a memory handle:
//  - associated allocator
//  - size of allocated handle
// handle_descriptor seeks to describe already-allocated handles and as such doesn't attempt to
// auto allocate/construct/free/destroy anything
//
// is_stateful = is this a stateful (instance) allocator or a global (singleton-style) allocator
// has_size = does this allocator track size of the allocated item
// is_singular = standard allocator behavior is to be able to allocate multiple items.  a singular allocator can only allocate one item
template <class TAllocator, bool is_stateful, bool has_size, bool is_singular>
class handle_descriptor_base;

namespace impl {


template <class TValue, bool is_present, TValue default_value>
class value_evaporator;

template <class TValue, TValue default_value>
class value_evaporator<TValue, true, default_value>
{

};

template <class TValue, TValue default_value>
class value_evaporator<TValue, false, default_value>
{
protected:
};

// https://en.cppreference.com/w/cpp/language/ebo we can have specialized base classes which are empty
// and don't hurt our sizing
template <class TAllocator, bool is_stateful>
class allocator_descriptor_base;


// TAllocator could be a ref here
template <class TAllocator>
class allocator_descriptor_base<TAllocator, true>
{
    TAllocator allocator;

protected:
    // NOTE: variadic would be nice, but obviously not always available
    template <class TAllocatorParameter>
    allocator_descriptor_base(TAllocatorParameter& p) :
        allocator(p) {}

    // Not unusual for a stateful allocator to default construct itself just
    // how we want it
    allocator_descriptor_base() {}

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;
    typedef typename allocator_type::handle_type handle_type;

    // Would be nice if we could const this, but for stateful allocators that's not reasonable
    allocator_type& get_allocator() { return allocator; }

    const allocator_type& get_allocator() const { return allocator; }

protected:
};


template <class TAllocator>
struct allocator_descriptor_base<TAllocator, false>
{
    typedef TAllocator allocator_type;

    // NOTE: odd, but OK.  Since we're stateless, we can return what otherwise
    // would be an invalid reference
    allocator_type& get_allocator() const { return TAllocator(); }
};

// singular technically doesn't track a handle
// TODO: refactor to utilize value_evaporator (get_allocator, too)
template <class TAllocator, bool is_singular>
class handle_descriptor_base;

template <class TAllocator>
class handle_descriptor_base<TAllocator, true>
{
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;

protected:
    handle_descriptor_base(bool) {}

    typedef typename allocator_type::handle_type handle_type;

    void handle(bool) {}

    value_type& lock(allocator_type& a, size_type pos = 0, size_type len = 0)
    {
        return a.lock(true, pos, len);
    }

    void unlock(allocator_type& a) { a.unlock(true); }

public:
    bool handle() const { return true; }
};


template <class TAllocator>
struct handle_descriptor_base<TAllocator, false>
{
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;

private:
    handle_type m_handle;

protected:
    void handle(const handle_type& h) { m_handle = h; }

    handle_descriptor_base(const handle_type& h) : m_handle(h) {}

    value_type& lock(allocator_type& a, size_type pos = 0, size_type len = 0)
    {
        return a.lock(m_handle, pos, len);
    }

    void unlock(allocator_type& a) { a.unlock(m_handle); }

public:
    bool handle() const { return m_handle; }
};


// base_base manages stateful+singular, but doesn't yet touch has_size
// singular allocators have a simplified handle model, basically true = successful/good handle
// false = bad/unallocated handle - we assume always good handle for this descriptor, so no
// handle is actually tracked
template <class TAllocator, bool is_stateful, bool is_singular>
class allocator_and_handle_descriptor_base :
        public allocator_descriptor_base<TAllocator, is_stateful>,
        public handle_descriptor_base<TAllocator, is_singular>
{
    typedef allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular> this_t;
    typedef allocator_descriptor_base<TAllocator, is_stateful> base_t;
    typedef handle_descriptor_base<TAllocator, is_singular> handle_base_t;

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename handle_base_t::handle_type handle_type;
    typedef estd::allocator_traits<allocator_type> allocator_traits;

public:
    // TODO: Still need to reconcile actually passing in handle for already-allocated-handle scenarios
    template <class TAllocatorParameter>
    allocator_and_handle_descriptor_base(TAllocatorParameter& p, const handle_type& h) :
            base_t(p),
            handle_base_t(h)
    {}

    allocator_and_handle_descriptor_base(const handle_type& h) :
        handle_base_t(h) {}


    // Indicates whether this handle can be invalid or not.
    // FALSE = invalid is possible (typical dynamic allocation behavior)
    // TRUE = invalid is impossible (fixed/singular behavior)
    static bool CONSTEXPR is_always_allocated() { return is_singular; }

    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return handle_base_t::lock(base_t::get_allocator(), pos, count);
    }

    // a necessary evil - since most STL-type operations reasonably are const'd for things
    // like 'size' etc, but underlying mechanisms which they call (this one in particular)
    // are not const when it comes to locking/stateful operations
    value_type& clock(size_type pos = 0, size_type count = 0) const
    {
        return const_cast<this_t*>(this)->lock();
    }

    void unlock() { handle_base_t::unlock(base_t::get_allocator()); }

    void cunlock() const { const_cast<this_t*>(this)->unlock(); }

    bool reallocate(size_type size)
    {
        handle_base_t::handle(base_t::get_allocator().reallocate(true, size));
        return handle_base_t::handle() != allocator_traits::invalid();
    }

    bool is_allocated() const
    {
        return allocator_traits::invalid() != handle_base_t::handle();
    }

    bool allocate(size_type n)
    {
        handle_base_t::handle(base_t::get_allocator().allocate(n));
        return is_allocated();
    }

    void free()
    {
        base_t::get_allocator().free(handle_base_t::handle());
    }
};




}


// With explicit size knowledge
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base<TAllocator, is_stateful, false, is_singular> :
        public impl::allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular>
{
    typedef impl::allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular> base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::allocator_traits allocator_traits;
    typedef typename base_t::handle_type handle_type;

private:
    size_type m_size;

protected:
    void size(size_type n) { m_size = n; }

    handle_descriptor_base(const handle_type& h) :
            base_t(h) {}

    template <class TAllocatorParameter>
    handle_descriptor_base(TAllocatorParameter& p, const handle_type& h) :
        base_t(p, h) {}

    handle_descriptor_base(allocator_type& allocator, const handle_type& h, size_type initial_size = 0) :
        base_t(allocator, h),
        m_size(initial_size)
    {}


public:
    size_type size() const { return m_size; }

    bool reallocate(size_type size)
    {
        m_size = size;
        return base_t::reallocate(size);
    }
};


// With implicit size knowledge (standard allocator model)
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base<TAllocator, is_stateful, true, is_singular> :
        public impl::allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular>
{
    typedef impl::allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular> base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename base_t::handle_type handle_type;

    handle_descriptor_base(const handle_type& h) : base_t(h) {}

    template <class TAllocatorParameter>
    handle_descriptor_base(TAllocatorParameter& a, const handle_type& h)
            : base_t(a, h) {}

    size_type size() const { return base_t::get_allocator().size(base_t::handle()); }
};


// Special-case handle descriptor who has a 1:1 parity with maximum
// and current allocated size.  Useful for allocators in a permanent const-mode
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_parity :
        public impl::allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular>
{
    typedef impl::allocator_and_handle_descriptor_base<TAllocator, is_stateful, is_singular> base_t;

public:
    typedef typename base_t::size_type size_type;
    typedef typename base_t::allocator_type allocator_type;

    // FIX: Harcoded to singular type
    // FIX: Need better name than 'T'
    template <class T>
    handle_descriptor_parity(const T& p) : base_t(p, true) {}

    size_type size() const { return base_t::get_allocator().max_size(); }

    bool reallocate(size_type size)
    {
        // Cannot issue a reallocate for this type of allocator
        assert(false);
    }
};


// NOTE: Pretty sure this isn't < C++11 friendly, so be sure to do explicit specializations
// for particular TAllocator varieties - will have to work out TAllocator& for those as well
template <class TAllocator,
          class TTraits = allocator_traits<typename remove_reference<TAllocator>::type> >
class handle_descriptor :
        public handle_descriptor_base<
            TAllocator,
            TTraits::is_stateful(),
            TTraits::has_size(),
            TTraits::is_singular() >
{
    typedef handle_descriptor_base<
            TAllocator,
            TTraits::is_stateful(),
            TTraits::has_size(),
            TTraits::is_singular() > base_t;

public:
    handle_descriptor() : base_t(TTraits::invalid()) {}

    template <class TAllocatorParameter>
    handle_descriptor(TAllocatorParameter& p) : base_t(p, TTraits::invalid()) {}

    typedef typename base_t::allocator_traits allocator_traits;
    typedef typename base_t::size_type size_type;

};


}}
