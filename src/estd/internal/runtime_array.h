#pragma once

#include "platform.h"
#include "../traits/allocator_traits.h"
#include "fwd/dynamic_array.h"
#include "container/fwd.h"
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
// Impl is usually some flavor of estd::internal::impl::dynamic_array
template <ESTD_CPP_CONCEPT(AllocatedArrayImpl) Impl>
class allocated_array :
#ifdef ARDUINO
    public print_handler_tag,
#endif
    public no_max_string_length_tag
{
public:
    using impl_type = Impl;
    // was going to go only allocator_type, but there has been a flip flop between favoring traits
    // or not in the c++ std, so keeping both
    using allocator_type = typename impl_type::allocator_type;
    using allocator_traits = typename impl_type::allocator_traits;

    //typedef typename allocator_traits::allocator_type allocator_type;
    typedef typename allocator_traits::handle_type handle_type;
    //typedef typename allocator_traits::handle_with_size handle_with_size;
    //typedef typename allocator_traits::reference reference; // one of our allocator_traits doesn't reveal this but I can't figure out which one

    using size_type = typename allocator_traits::size_type;
    using value_type = typename allocator_traits::value_type;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    typedef typename allocator_traits::allocator_valref allocator_valref;

    typedef dynamic_array_helper<impl_type> helper;

    using policy_type = typename impl_type::policy_type;

    friend struct dynamic_array_helper<impl_type>;

protected:
    // NOTE: It's conceivable we could use a value_evaporator here in situations where
    // allocated array pointed to a static const(expr) * of some kind
    Impl m_impl;

public:
    static bool constexpr is_locking = internal::has_locking_tag<allocator_type>::value;

    // DEBT: a_it = allocator_iterator, fix up name after we formalize or phase out
    // FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
    using a_it = typename allocator_traits::iterator;

    // Always try to avoid explicit locking and unlocking ... but sometimes
    // you gotta do it, so these are public
    constexpr pointer lock(size_type pos = 0, size_type count = 0)
    {
        return &m_impl.lock(pos, count);
    }

    constexpr const_pointer clock(size_type pos = 0, size_type count = 0) const
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
    // DEBT: Switch this out for a forwarding constructor and do this down
    // at m_impl ctor level - thus opening up more comfortable constexpr
    // possibilities
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
    ESTD_CPP_CONSTEXPR_RET EXPLICIT allocated_array(const THelperParam& p) :
            m_impl(p) {}

    // DEBT: Use ESTD_CPP_FORWARDING_CTOR_MEMBER if we can
    // DEBT: Consider requiring in_place_t here
    // DEBT: Use && here, not doing so because a complex failure with impl::dynamic_array occurs
    // and at the moment we are only adding this for true constexpr init on AVR (which is tricky
    // to do with initializer_list)
#if __cpp_variadic_templates
    template <class ...T>
    constexpr explicit allocated_array(T...t) :
        m_impl(t...)
    {}
#endif

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
    using accessor = typename a_it::accessor;
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

    ///
    /// @param s incoming sequence to find
    /// @param pos first position in 'this' to evaluate from
    /// @param count length of incoming 's'
    /// @return
    ESTD_CPP_CONSTEXPR(14) size_type find(
        const_pointer s, size_type pos, size_type count, size_type npos) const
    {
        // FIX: pos treatment is all wrong, we have to START searching from pos
        if (pos == npos)    pos = size();   // DEBT: I think this line should go back up to string

        // if our length is less than requested string, we'll never match anyway
        // so abort
        if(pos < count) return npos;

        pos -= count;

        const_pointer data = clock();
        const_pointer end = data + pos + 1;

        for(const_pointer i = data; i != end; ++i)
        {
            if(memcmp(s, i, count) == 0)
            {
                cunlock();
                return i - data;
            }
        }

        cunlock();

        return npos;
    }


    ///
    /// @param s incoming sequence to find
    /// @param pos last position in 'this' to evaluate from
    /// @param count length of incoming 's'
    /// @return
    ESTD_CPP_CONSTEXPR(14) size_type rfind(
        const_pointer s, size_type pos, size_type count, size_type npos) const
    {
        // FIX: "The search begins at pos" always
        if(pos == npos) pos = size();   // DEBT: I think this line should go back up to string

        // if our length is less than requested string, we'll never match anyway
        // so abort
        if(pos < count) return npos;

        pos -= count;

        const_pointer data = clock();
        const_pointer rend = data - 1;

        data += pos;

        for(; data != rend; --pos, --data)
        {
            if(memcmp(s, data, count) == 0)
            {
                cunlock();
                return pos;
            }
        }

        cunlock();

        return npos;
    }

    ESTD_CPP_CONSTEXPR(14) size_type find_last_of(
        const_pointer s, size_type pos, size_type count, size_type npos)
    {
        if(pos == npos) pos = size();   // DEBT: I think this line should go back up to string

        const_pointer data = clock();
        const_pointer rend = data - 1;

        data += pos;
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
