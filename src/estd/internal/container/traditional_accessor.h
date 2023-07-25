#pragma once

#include "../platform.h"
#include "locking_accessor.h"
#include "handle_with_offset.h"

namespace estd { namespace internal {

// DEBT: Pretty sure there's a specialized handle_with_offset or similar which is single-pointer specific,
// i.e. 100% overlaps with this.  Look around and attempt to KEEP traditional_accessor and derive other
// from us here
// DEBT: I have a sneaky suspicion I am off down a rabbit hole here, probably a bare reference would do
// most of this... ?  Stopping the madness ... for now
template <class T>
struct traditional_accessor
{
    ESTD_CPP_STD_VALUE_TYPE(T);

    // DEBT: This needs to be a trait somewhere
    // experimental flag reflecting that this memory block will never move
    static CONSTEXPR bool is_pinned = true;

    pointer p;

    typedef traditional_accessor this_type;

    /*
    struct h_exp_
    {
        pointer& p;

        void increment(int v = 1) { p += v; }

        h_exp_& operator +=(int v)
        {
            increment(v);
            return *this;
        }

        bool operator==(const h_exp_& compare_to) const
        {
            return p == compare_to.p;
        }
    };  */

    ESTD_CPP_CONSTEXPR_RET EXPLICIT traditional_accessor(reference v) : p(&v) {}

    template <class TAllocator>
    ESTD_CPP_CONSTEXPR_RET traditional_accessor(TAllocator& a,
        const typename estd::allocator_traits<TAllocator>::handle_with_offset& h) :
        p(&a.lock(h))
    {
    }

    /*
    traditional_accessor& h_exp() { return *this; }
    const traditional_accessor& h_exp() const { return *this; }*/
    pointer& h_exp() { return p; }

    const pointer& h_exp() const { return p; }

    operator value_type&() { return *p; }
    explicit operator value_type&() const { return *p; }

    value_type& lock() { return *p; }
    const value_type& clock() const { return *p; }
    void unlock() {}

    // DEBT: I think these arrows need some filtering
    reference operator->() { return *p; }
    const_reference operator->() const { return *p; }

    traditional_accessor& operator=(const value_type& copy_from)
    {
        *p = copy_from;
        return *this;
    }

    bool operator==(const_reference& compare_to) const
    {
        return *p == compare_to;
    }

    bool operator==(const this_type& compare_to) const
    {
        return p == compare_to.p;
    }

    bool operator>=(const this_type& compare_to) const
    {
        return p >= compare_to.p;
    }

    bool operator<=(const this_type& compare_to) const
    {
        return p <= compare_to.p;
    }
};

/*
template <class T>
ptrdiff_t operator-(const traditional_accessor<T>& lhs, const traditional_accessor<T>& rhs)
{
    return lhs.p - rhs.p;
}
*/

template <class T>
traditional_accessor<T> operator-(const traditional_accessor<T>& lhs, const T& rhs)
{
    return lhs.p - rhs;
}

template <class T>
bool operator==(const T& lhs, const traditional_accessor<T>& rhs)
{
    return lhs == rhs.clock();
}


}}