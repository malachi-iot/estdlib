#pragma once

#include "../internal/impl/handle_desc.h"
#ifdef FEATURE_STD_CASSERT
#include <cassert>
#endif

// TODO: Consider organizing this under internal/handle_desc since even though this is all
// very allocator oriented, it doesn't in it of itself define an allocator
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
             TTraits::is_contiguous_exp>
{
     typedef internal::handle_descriptor_base<
             TAllocator,
             TTraits::is_stateful_exp,
             TTraits::has_size_exp,
             TTraits::is_singular_exp,
             TTraits::is_contiguous_exp> base_t;

public:
     handle_descriptor(typename TTraits::handle_type h = TTraits::invalid()) :
         base_t(h) {}

     template <class TAllocatorParameter>
     handle_descriptor(TAllocatorParameter& p) : base_t(p, TTraits::invalid()) {}
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
