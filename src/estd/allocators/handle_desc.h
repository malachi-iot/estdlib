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

    allocator_type get_allocator() const { return TAllocator(); }
};

// singular technically doesn't track a handle
// TODO: refactor to utilize value_evaporator (get_allocator, too)
template <class TAllocator, bool is_singular>
class handle_descriptor_base;

template <class TAllocator>
class handle_descriptor_base<TAllocator, true>
{
protected:
    void handle(bool) {}

public:
    typedef bool handle_type;

    bool handle() const { return true; }
};


template <class TAllocator>
struct handle_descriptor_base<TAllocator, false>
{
    typedef typename remove_reference<TAllocator>::type::handle_type handle_type;

private:
    handle_type m_handle;

protected:
    void handle(const handle_type& h) { m_handle = h; }

    handle_descriptor_base(const handle_type& h) : m_handle(h) {}

public:
    bool handle() const { return m_handle; }
};



}

// base_base manages stateful+singular, but doesn't yet touch has_size
// singular allocators have a simplified handle model, basically true = successful/good handle
// false = bad/unallocated handle - we assume always good handle for this descriptor, so no
// handle is actually tracked
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base_base :
        public impl::allocator_descriptor_base<TAllocator, is_stateful>,
        public impl::handle_descriptor_base<TAllocator, is_singular>
{
    typedef handle_descriptor_base_base<TAllocator, is_stateful, is_singular> this_t;
    typedef impl::allocator_descriptor_base<TAllocator, is_stateful> base_t;
    typedef impl::handle_descriptor_base<TAllocator, is_singular> handle_base_t;

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef allocator_traits<allocator_type> allocator_traits;

public:
    template <class TAllocatorParameter>
    handle_descriptor_base_base(TAllocatorParameter& p) :
        base_t(p) {}

    handle_descriptor_base_base() {}

    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return base_t::get_allocator().lock(handle_base_t::handle(), pos, count);
    }

    // a necessary evil - since most STL-type operations reasonably are const'd for things
    // like 'size' etc, but underlying mechanisms which they call (this one in particular)
    // are not const when it comes to locking/stateful operations
    value_type& clock(size_type pos = 0, size_type count = 0) const
    {
        return const_cast<this_t*>(this)->lock();
    }

    void unlock() { base_t::get_allocator().unlock(handle_base_t::handle()); }

    void cunlock() const { const_cast<this_t*>(this)->unlock(); }

    bool reallocate(size_type size)
    {
        handle_base_t::handle(base_t::get_allocator().reallocate(true, size));
        return handle_base_t::handle() != allocator_traits::invalid();
    }
};



// With explicit size knowledge
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base<TAllocator, is_stateful, false, is_singular> :
        public handle_descriptor_base_base<TAllocator, is_stateful, is_singular>
{
    typedef handle_descriptor_base_base<TAllocator, is_stateful, is_singular> base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::allocator_traits allocator_traits;

private:
    size_type m_size;

protected:
    void size(size_type n) { m_size = n; }

    handle_descriptor_base() {}

    template <class TAllocatorParameter>
    handle_descriptor_base(TAllocatorParameter& p) :
        base_t(p) {}

    handle_descriptor_base(allocator_type& allocator, size_type initial_size = 0) :
        base_t(allocator),
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


// WITH implicit size knowledge (standard allocator model)
// TBD
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base<TAllocator, is_stateful, true, is_singular> :
        impl::allocator_descriptor_base<TAllocator, is_stateful>,
        impl::handle_descriptor_base<TAllocator, is_singular>
{
    typedef impl::allocator_descriptor_base<TAllocator, is_stateful> base_t;
    typedef impl::handle_descriptor_base<TAllocator, is_singular> handle_base_t;
};


// Special-case handle descriptor who has a 1:1 parity with maximum
// and current allocated size.  Useful for allocators in a permanent const-mode
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_parity :
        handle_descriptor_base_base<TAllocator, is_stateful, is_singular>
{
    typedef handle_descriptor_base_base<TAllocator, is_stateful, is_singular> base_t;

public:
    typedef typename base_t::size_type size_type;

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
          class TTraits = allocator_traits<typename remove_reference<TAllocator>::type>>
class handle_descriptor :
        public handle_descriptor_base<
            TAllocator,
            TTraits::is_stateful(),
            false,
            TTraits::is_singular() >
{
    typedef handle_descriptor_base<
            TAllocator,
            TTraits::is_stateful(),
            false,
            TTraits::is_singular() > base_t;

public:
    handle_descriptor() {}

    handle_descriptor(TAllocator& a) : base_t(a) {}

    template <class TAllocatorParameter>
    handle_descriptor(TAllocatorParameter& p) : base_t(p) {}
};


}}
