#pragma once

#include "../type_traits.h"

// Kind of a 2nd attempt at estd::internal::accessor

namespace estd { namespace internal {

template <class Impl>
struct locking_accessor
{
    typedef Impl impl_type;
    typedef locking_accessor this_type;

    // This is type for moving pointer/handle around (for iterator use)
    typedef typename impl_type::offset_type offset_type;
    typedef typename impl_type::const_offset_type const_offset_type;
    typedef typename impl_type::value_type value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    impl_type impl_;

    // DEBT: Should be something like 'handle_type', not 'reference'
    //ESTD_CPP_CONSTEXPR_RET EXPLICIT locking_accessor(reference r) : impl_(r) {}
    template <class ...Args>
    constexpr explicit locking_accessor(Args&&...args) :
        impl_(std::forward<Args>(args)...) {}

    offset_type h_exp() { return impl_.offset(); }
    const_offset_type h_exp() const { return impl_.offset(); }

    reference lock() { return impl_.lock(); }
    const_reference clock() const { return impl_.lock(); }
    void unlock() { return impl_.unlock(); }
    void cunlock() const { return impl_.unlock(); }

    // DEBT: Needs filtering
    // DEBT: Leaves unlocked!
    reference operator->() { return lock(); }
    const_reference operator->() const { return clock(); }

    // DEBT: Leaves unlocked!
    operator reference() { return lock(); }
    explicit operator reference() const { return lock(); }

    this_type& operator=(const value_type& copy_from)
    {
        lock() = copy_from;
        unlock();
        return *this;
    }

    bool operator==(const_reference& compare_to) const
    {
        bool r = clock() == compare_to;
        cunlock();
        return r;
    }

    // DEBT: This needs to be a trait somewhere
    // experimental flag reflecting that this memory block will never move
    static CONSTEXPR bool is_pinned = true;
};

}}