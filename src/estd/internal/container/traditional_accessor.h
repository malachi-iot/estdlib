#pragma once

#include "../platform.h"
#include "locking_accessor.h"
#include "handle_with_offset.h"

// DEBT: Pretty sure there's a specialized handle_with_offset or similar which is single-pointer specific,
// i.e. 100% overlaps with this.  Look around and attempt to KEEP traditional_accessor and derive other
// from us here

namespace estd { namespace internal {

namespace impl {

template <class T>
struct traditional_accessor
{
    ESTD_CPP_STD_VALUE_TYPE(T)

private:
    pointer p;

public:
    typedef pointer& offset_type;
    typedef const pointer& const_offset_type;
    typedef reference locked_type;
    typedef const_reference const_locked_type;

    template <class Allocator>
    ESTD_CPP_CONSTEXPR_RET EXPLICIT traditional_accessor(Allocator& a,
        const typename allocator_traits<Allocator>::handle_with_offset& h) :
        p(&a.lock(h))
    {
    }

    ESTD_CPP_CONSTEXPR_RET EXPLICIT traditional_accessor(reference v) :
        p(&v) {}

    ESTD_CPP_CONSTEXPR(17) offset_type offset() { return p; }
    constexpr const_offset_type offset() const { return p; }

    ESTD_CPP_CONSTEXPR(17) reference lock() { return *p; }
    constexpr const_reference lock() const { return *p; }
    static ESTD_CPP_CONSTEVAL void unlock() {}
};

}

template <class T>
struct traditional_accessor : locking_accessor<impl::traditional_accessor<T> >
{
    typedef locking_accessor<impl::traditional_accessor<T> > base_type;

    // DEBT: Consider strongly making forwarding macro explicit always
    //ESTD_CPP_FORWARDING_CTOR(traditional_accessor)
    //ESTD_CPP_CONSTEXPR_RET traditional_accessor(T& v) : base_type(v) {}

#if __cpp_variadic_templates
    template <class ...Args>
    explicit constexpr traditional_accessor(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}
#endif

    traditional_accessor& operator=(const typename base_type::value_type& copy_from)    // NOLINT
    {
        base_type::operator=(copy_from);
        return *this;
    }
};


/*
 * 13MAY25 MB - These seem like an awful idea.  Commenting out. Maybe my memory will jog
 * why these were needed.  Technically a breaking change
// DEBT: Move this up to locking_accessor area
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
*/


}}
