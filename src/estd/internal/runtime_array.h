#pragma once

#include "platform.h"
#include "../traits/allocator_traits.h"
#include "container/accessor.h"
#include "container/iterator.h"
#include "container/specializer.h"
#include "container/traditional_accessor.h"
#include "../algorithm.h"
#include "../initializer_list.h"
#include "iterator_standalone.h"
#include "container/starts_with.h"

// Intermediate flag which more or less disables all the handle/locking mechanisms
// in favor of traditional pointer-style iterator.  Development flag only, once it
// works, the compile time flag will be deduced by querying the allocator itself
#define FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL 0

namespace estd { namespace internal {

// experimental and not good
// maxStringLength needs a clue to not inspect a type for length so that
// we don't get 'not implemented' problems with SFINAE
struct no_max_string_length_tag {};

// base class for any 'allocated' array, even ones using a fixed allocator
// which pushes the boundaries of what allocated even really means
// importantly, this allocated_array doesn't provide for growing/shrinking the array,
// which means things like insert, append, erase are also not present.  Look to
// dynamic array for that
// TImpl is usually some flavor of estd::internal::impl::dynamic_array
template <class TImpl>
class allocated_array :
#ifdef ARDUINO
    public print_handler_tag,
#endif
    public no_max_string_length_tag
{
public:
    typedef TImpl impl_type;
    typedef typename impl_type::allocator_type allocator_type;
    typedef typename impl_type::allocator_traits allocator_traits;
    typedef typename allocator_traits::handle_type handle_type;
    //typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::pointer pointer;
    //typedef typename allocator_traits::reference reference; // one of our allocator_traits doesn't reveal this but I can't figure out which one
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    typedef typename allocator_type::value_type value_type;
    typedef const value_type* const_pointer;

    typedef typename allocator_traits::allocator_valref allocator_valref;

    typedef dynamic_array_helper<impl_type> helper;

    friend class dynamic_array_helper<impl_type>;

protected:
    // NOTE: It's conceivable we could use a value_evaporator here in situations where
    // allocated array pointed to a static const(expr) * of some kind
    TImpl m_impl;

public:
    static bool CONSTEXPR is_locking = internal::has_locking_tag<allocator_type>::value;

    // DEBT: a_it = allocator_iterator, fix up name after we formalize or phase out
    // FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
    typedef typename allocator_traits::iterator a_it;

    // Always try to avoid explicit locking and unlocking ... but sometimes
    // you gotta do it, so these are public
    value_type* lock(size_type pos = 0, size_type count = 0)
    {
        return &m_impl.lock(pos, count);
    }

    const value_type* clock(size_type pos = 0, size_type count = 0) const
    {
        return &m_impl.clock(pos, count);
    }

    void unlock()
    {
        m_impl.unlock();
    }

    void cunlock() const
    {
        m_impl.cunlock();
    }

protected:
    explicit allocated_array(allocator_type& alloc) :
        m_impl(alloc) {}

    allocated_array(const allocated_array& copy_from)
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
        = default;
#else
        : m_impl(copy_from.m_impl)
    {
    }
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    allocated_array(std::initializer_list<value_type> initlist)
    {
        pointer p = lock();

        estd::copy(initlist.begin(), initlist.end(), p);

        m_impl.size(initlist.size());

        unlock();
    }
#endif

    handle_with_offset offset(size_type  pos) const
    {
        //return helper.get_allocator().offset(helper.handle(), pos);
        return m_impl.offset(pos);
    }

    // internal version of replace not conforming to standard
    // (standard version also inserts or removes characters if requested,
    //  this one ONLY replaces the entire buffer)
    // TODO: change to assign
    void assign(const value_type* buf, size_type len)
    {
        m_impl.copy_into(buf, 0, len);
    }


    template <class TForeignImpl>
    ESTD_CPP_CONSTEXPR_RET bool starts_with(const allocated_array<TForeignImpl>& compare_to) const
    {
        return helper::starts_with(*this, compare_to);
    }

public:
    ESTD_CPP_DEFAULT_CTOR(allocated_array)

    template <class THelperParam>
    allocated_array(const THelperParam& p) :
            m_impl(p) {}

#if UNUSED
    // TODO: make accessor do this comparison in a self contained way
    // allocator itself only needs to be stateful if it needs to do handle locking
    // if not, then we assume it's pointer based and thusly can access the item
    // without an allocator pointer, stateful or otherwise
    // TODO: Move this accessor typedef out to helper, who can lean slightly less on
    // constexpr to get the job done
#ifdef FEATURE_CPP_CONSTEXPR
    typedef typename estd::conditional<
#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                allocator_traits::is_stateful() && allocator_traits::is_locking(),
#else
                allocator_traits::is_stateful_exp && allocator_traits::is_locking_exp,
#endif
                estd::internal::accessor<allocator_type>,
                estd::internal::accessor_stateless<allocator_type> >::type
                handle_accessor;
#else
    // pre C++11 tricky to optimize this way
    typedef estd::internal::accessor<allocator_type> accessor;
#endif
#endif

    // Use this only when we're certain no locking/handle tracking is required (which
    // is 99%+ of the time, as of this writing)
    // DEBT: Surely this mechanism is already present elsewhere
    //typedef traditional_accessor<value_type> accessor;
#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
    typedef value_type& accessor;

    accessor create_accessor(unsigned o = 0) const
    {
        accessor v(get_allocator().lock(offset(o)));
        return v;
    }
#else
    typedef typename a_it::accessor accessor;
    //typedef handle_accessor accessor;
    //typedef traditional_accessor<value_type> accessor;

    //ESTD_CPP_CONSTEXPR_RET
    accessor create_accessor(unsigned o = 0) const
    {
        // DEBT: using temp variable here because accessor ctor signature REQUIRES a reference,
        // even for stateless allocators
        allocator_valref allocator = get_allocator();
        return accessor(allocator, offset(o));
    }
#endif

    typedef pointer traditional_iterator;

    ESTD_CPP_CONSTEXPR_RET size_type size() const { return m_impl.size(); }

    allocator_valref get_allocator() const
    {
        // FIX: Hate this, but since we have stateful allocators flying around everywhere, even
        // embedded within dynamic array, we are forced to do this so that we conform to std::get_allocator
        // const call (important because we need to conform to const for front, back, etc)
        // because we need locking, side affects are to be expected
        return const_cast<impl_type&>(m_impl).get_allocator();
    }

#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
    typedef traditional_iterator iterator;

    traditional_iterator create_iterator(unsigned o = 0) const
    {
        auto& v = get_allocator().lock(offset(o));
        return traditional_iterator{&v};
    }
#else
    typedef a_it iterator;

    iterator create_iterator(unsigned o = 0) const
    {
        return iterator(create_accessor(o));
    }
#endif

    typedef const iterator const_iterator;

    iterator begin()
    {
        return create_iterator();
    }

    const_iterator cbegin() const
    {
        return create_iterator();
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    iterator end()
    {
        return create_iterator(size());
        /*
        handle_with_offset o = offset(size());

        return iterator(get_allocator(), o); */
    }


    const_iterator cend() const
    {
        return create_iterator(size());
        /*
        handle_with_offset o = offset(size());

        return iterator(get_allocator(), o); */
    }

    const_iterator end() const
    {
        return cend();
    }

    accessor at(size_type pos) const
    {
        // DEBT: Consider routing this *through* iterator just to dogfood and DRY a bit
        // TODO: place in error/bounds checking runtime ability within
        // accessor itself.  perhaps wrap it all up in a FEATURE_ESTD_BOUNDSCHECK
        // to compensate for lack of exceptions requiring additional
        // data in accessor itself
        //return accessor(get_allocator(), offset(pos));
        return create_accessor(pos);
    }


    accessor operator[](size_type pos) const
    {
        //return accessor(get_allocator(), offset(pos));
        return create_accessor(pos);
    }

    accessor front() const
    {
        return create_accessor(0);
    }

    accessor back() const
    {
        return create_accessor(size() - 1);
    }

    ESTD_CPP_CONSTEXPR_RET size_type max_size() const
    {
        return m_impl.max_size();
    }

    ESTD_CPP_CONSTEXPR_RET bool empty() const
    {
        return m_impl.empty();
    }

    // Non standard and not yet functional
    ESTD_CPP_CONSTEXPR_RET bool full() const
    {
        return m_impl.full();
    }

    // copy (into dest)
    // officially only for basic_string, but lives happily here in allocated_array
    size_type copy(typename estd::remove_const<value_type>::type* dest,
                   size_type count, size_type pos = 0) const
    {
        return helper::copy_to(*this, dest, count, pos);
    }

    template <class TForeignImpl>
    bool operator ==(const allocated_array<TForeignImpl>& compare_to) const
    {
        size_type raw_size = size();
        size_type s_size = compare_to.size();

        if(raw_size != s_size) return false;

        return helper::equal(*this, compare_to, raw_size);
    }
};

// base class for dynamic_array
// performs a lot of dynamic_array duties, but behaves as a read-only entity
// though constness is not specifically enforced, as writable entities will
// derive from this
template <class THelper>
class runtime_array
{
public:
};

}

/*
template <class Impl>
void swap(typename internal::allocated_array<Impl>::handle_accessor lhs,
    typename internal::allocated_array<Impl>::handle_accessor rhs)
{
    typedef typename internal::allocated_array<Impl>::value_type& reference;

    reference l = lhs.lock();
    reference r = rhs.lock();

    swap(l, r);

    rhs.unlock();
    lhs.unlock();
}   */

template <class Allocator>
void swap(typename internal::accessor<Allocator> lhs,
    typename internal::accessor<Allocator> rhs)
{
    typedef typename Allocator::value_type& reference;

    reference l = lhs.lock();
    reference r = rhs.lock();

    swap(l, r);

    lhs.unlock();
    rhs.unlock();
}

}
