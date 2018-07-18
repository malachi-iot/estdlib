#pragma once

#include "../internal/impl/handle_desc.h"
#ifdef FEATURE_STD_CASSERT
#include <cassert>
#endif

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
// is_contiguous = true = normal behavior, false = alternative allocator behavior where locks
//                 don't always provide entire range
template <class TAllocator,
          bool is_stateful,
          bool has_size,
          bool is_singular,
          bool is_contiguous>
class handle_descriptor_base;




// common base class
// size managed by classes derived from this one
// singular allocators have a simplified handle model, basically true = successful/good handle
// false = bad/unallocated handle - we assume always good handle for this descriptor, so no
// handle is actually tracked
template <class TAllocator, bool is_stateful, bool is_singular,
          class TTraits = estd::allocator_traits<TAllocator> >
class allocator_and_handle_descriptor :
        public impl::allocator_descriptor<TAllocator, is_stateful>,
        public impl::handle_descriptor<TAllocator, is_singular>,
        public impl::contiguous_descriptor<TAllocator, true> // hard-wired to contiguous, for now
{
    typedef allocator_and_handle_descriptor<TAllocator, is_stateful, is_singular> this_t;
    typedef impl::allocator_descriptor<TAllocator, is_stateful> base_t;
    typedef impl::handle_descriptor<TAllocator, is_singular> handle_base_t;
    typedef impl::contiguous_descriptor<TAllocator, true> contiguous_base;

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename handle_base_t::handle_type handle_type;
    typedef TTraits allocator_traits;

public:
    // TODO: Still need to reconcile actually passing in handle for already-allocated-handle scenarios
    template <class TAllocatorParameter>
    allocator_and_handle_descriptor(TAllocatorParameter& p, const handle_type& h) :
            base_t(p),
            handle_base_t(h)
    {}

    allocator_and_handle_descriptor(const handle_type& h) :
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


    void copy_into(const value_type* source, size_type pos, size_type len)
    {
        contiguous_base::copy_into(
                base_t::get_allocator(),
                handle_base_t::handle(),
                source, pos, len);
    }

    bool reallocate(size_type size)
    {
        handle_base_t::handle(base_t::get_allocator().reallocate(handle_base_t::handle(), size));
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



// With explicit size knowledge
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base<TAllocator, is_stateful, false, is_singular, true> :
        public allocator_and_handle_descriptor<TAllocator, is_stateful, is_singular>
{
    typedef allocator_and_handle_descriptor<TAllocator, is_stateful, is_singular> base_t;

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
            base_t(h),
            m_size(0) {}

    template <class TAllocatorParameter>
    handle_descriptor_base(TAllocatorParameter& p, const handle_type& h) :
        base_t(p, h),
        m_size(0)
    {}

    handle_descriptor_base(allocator_type& allocator, const handle_type& h, size_type initial_size = 0) :
        base_t(allocator, h),
        m_size(initial_size)
    {}


public:
    size_type size() const { return m_size; }

    bool allocate(size_type size)
    {
        m_size = size;
        return base_t::allocate(size);
    }

    bool reallocate(size_type size)
    {
        m_size = size;
        return base_t::reallocate(size);
    }
};


// With implicit size knowledge - allocator can be queried for
// size of our tracked handle
template <class TAllocator, bool is_stateful, bool is_singular>
class handle_descriptor_base<TAllocator, is_stateful, true, is_singular, true> :
        public allocator_and_handle_descriptor<TAllocator, is_stateful, is_singular>
{
    typedef allocator_and_handle_descriptor<TAllocator, is_stateful, is_singular> base_t;

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


}

// Mainly done to be pre c++11 friendly, but could eventually be a cleaner architecutre also
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
template <class TAllocator,
           class TTraits = allocator_traits<typename remove_reference<TAllocator>::type> >
class handle_descriptor :
         public internal::handle_descriptor_base<
             TAllocator,
             TTraits::is_stateful_exp,
             TTraits::has_size_exp,
             TTraits::is_singular_exp,
             true>
{
     typedef internal::handle_descriptor_base<
             TAllocator,
             TTraits::is_stateful_exp,
             TTraits::has_size_exp,
             TTraits::is_singular_exp,
             true> base_t;

public:
     handle_descriptor() : base_t(TTraits::invalid()) {}

     template <class TAllocatorParameter>
     handle_descriptor(TAllocatorParameter& p) : base_t(p, TTraits::invalid()) {}

     typedef typename base_t::allocator_traits allocator_traits;
     typedef typename base_t::size_type size_type;

};
#else
#ifdef FEATURE_CPP_CONSTEXPR
// NOTE: Pretty sure this isn't < C++11 friendly, so be sure to do explicit specializations
// for particular TAllocator varieties - will have to work out TAllocator& for those as well
template <class TAllocator,
          class TTraits = allocator_traits<typename remove_reference<TAllocator>::type> >
class handle_descriptor :
        public internal::handle_descriptor_base<
            TAllocator,
            TTraits::is_stateful(),
            TTraits::has_size(),
            TTraits::is_singular(),
            true>
{
    typedef internal::handle_descriptor_base<
            TAllocator,
            TTraits::is_stateful(),
            TTraits::has_size(),
            TTraits::is_singular(),
            true> base_t;

public:
    handle_descriptor() : base_t(TTraits::invalid()) {}

    template <class TAllocatorParameter>
    handle_descriptor(TAllocatorParameter& p) : base_t(p, TTraits::invalid()) {}

    typedef typename base_t::allocator_traits allocator_traits;
    typedef typename base_t::size_type size_type;

};
#else
template <class TAllocator,
        class TTraits = allocator_traits<typename remove_reference<TAllocator>::type> >
class handle_descriptor;
#endif

#endif
}
