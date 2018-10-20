#pragma once

#include "../../traits/allocator_traits.h"
#include "../value_evaporator.h"

// this is the low-level specialized handle_descriptor
// which only tracks handle itself - the 'full' handle descriptor
// also tracks size (see allocators/handle_desc)
namespace estd { namespace internal { namespace impl {




// contains no members, just specialized copy helpers
template <class TAllocator, bool is_contiguous>
struct contiguous_descriptor;


// experimental, unused, untested, but simple and useful enough I think it will graduate quickly
// no bounds checks are performed, as this is expected to be done by querying largest contiguous chunk size
template <class TAllocator>
struct contiguous_descriptor<TAllocator, true>
{
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename estd::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;
    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;

    // copy outside buffer into this handle-based memory
    static void copy_into(allocator_type& a, handle_type h, const value_type* source, size_type pos, size_type len)
    {
        value_type* dest = &allocator_traits::lock(a, h, pos, len);

        while(len--) *dest++ = *source++;

        allocator_traits::unlock(a, h);
    }


    // copy this handle-based memory to outside buffer
    static void copy_from(const allocator_type& a, handle_type h, value_type* dest, size_type pos, size_type len)
    {
        const value_type* source = &allocator_traits::clock(a, h, pos, len);

        while(len--) *dest++ = *source++;

        allocator_traits::cunlock(a, h);
    }
};

// https://en.cppreference.com/w/cpp/language/ebo we can have specialized base classes which are empty
// and don't hurt our sizing
template <class TAllocator, bool is_stateful>
class allocator_descriptor;


// TAllocator could be a ref here
template <class TAllocator>
class allocator_descriptor<TAllocator, true>
{
    TAllocator allocator;

protected:
    // NOTE: variadic would be nice, but obviously not always available
    template <class TAllocatorParameter>
    allocator_descriptor(TAllocatorParameter& p) :
        allocator(p) {}

    // Not unusual for a stateful allocator to default construct itself just
    // how we want it
    allocator_descriptor() {}

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;
    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;

    // Would be nice if we could const this, but for stateful allocators that's not reasonable
    allocator_type& get_allocator() { return allocator; }

    const allocator_type& get_allocator() const { return allocator; }

protected:
    typedef allocator_type& allocator_ref;
};


template <class TAllocator>
struct allocator_descriptor<TAllocator, false>
{
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename estd::allocator_traits<allocator_type> allocator_traits;
    typedef allocator_type allocator_ref;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_traits::handle_type handle_type;

    // NOTE: odd, but OK.  Since we're stateless, we can return what otherwise
    // would be an invalid reference.  We have to pull this stunt since
    // consumers of get_allocator() won't take an rvalue
    allocator_type& get_allocator() const
    {
        allocator_type* _fake = NULLPTR;
        return *_fake;
        //return allocator_type();
    }

protected:

};


template <class TAllocator, bool is_singular,
        class TTraits = estd::allocator_traits<typename remove_reference<TAllocator>::type> >
class handle_descriptor :
        value_evaporator<typename TTraits::handle_type, !is_singular, bool, true>
{
    typedef TTraits allocator_traits;
    typedef typename allocator_traits::value_type value_type;
    typedef typename allocator_traits::size_type size_type;

protected:
    typedef typename allocator_traits::handle_type handle_type;
    typedef typename allocator_traits::allocator_type allocator_type;

    typedef value_evaporator<handle_type, !is_singular, bool, true> base_t;

public:
    handle_type handle() const { return base_t::value(); }

protected:
    void handle(const handle_type& h) { base_t::value(h); }

    handle_descriptor(const handle_type& h) : base_t(h) {}

    value_type& lock(allocator_type& a, size_type pos = 0, size_type len = 0)
    {
        return allocator_traits::lock(a, handle(), pos, len);
    }

    const value_type& clock(const allocator_type& a, size_type pos = 0, size_type len = 0) const
    {
        return allocator_traits::clock(a, handle(), pos, len);
    }

    void unlock(allocator_type& a)
    {
        allocator_traits::unlock(a, handle());
    }

    void cunlock(const allocator_type& a) const
    {
        allocator_traits::cunlock(a, handle());
    }
};



}

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
    const value_type& clock(size_type pos = 0, size_type count = 0) const
    {
        return handle_base_t::clock(base_t::get_allocator(), pos, count);
    }

    void unlock() { handle_base_t::unlock(base_t::get_allocator()); }

    void cunlock() const { handle_base_t::cunlock(base_t::get_allocator()); }

#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    void construct(size_type pos, TArgs...args)
    {
        allocator_type& a = base_t::get_allocator();
        allocator_traits::construct(a, &lock(pos, 1), std::forward<TArgs>(args)...);
        unlock();
    }
#endif

    void destroy(size_type pos)
    {
        allocator_type& a = base_t::get_allocator();
        allocator_traits::destroy(a, &lock(pos, 1));
        unlock();
    }

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

}}
