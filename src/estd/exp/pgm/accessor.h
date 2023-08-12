#pragma once

#include "fwd.h"
#include "read.h"

namespace estd {

namespace experimental {

template <class T, bool near = true>
class pgm_accessor;

// "legacy" flavor, phasing out
template <class T>
class pgm_accessor<T> : protected internal::impl::pgm_allocator_traits<T, internal::variant_npos()>
{
    using base_type = internal::impl::pgm_allocator_traits<T, internal::variant_npos()>;
protected:
    using typename base_type::const_pointer;
    using typename base_type::value_type;

    const_pointer p;

    value_type value() const { return internal::pgm_read<T>(p); }

#if FEATURE_ESTD_PGM_EXP_IT
    const bool is_null() const
    {
        return p == nullptr || value() == 0;
    }
#endif

public:
    constexpr pgm_accessor(const_pointer p) : p{p} {}

    value_type operator*() const { return value(); }

    //constexpr
    bool operator==(const pgm_accessor& compare_to) const
    {
#if FEATURE_ESTD_PGM_EXP_IT
        if(is_null() && compare_to.is_null()) return true;
#endif
        return p == compare_to.p;
    }

    //constexpr
    bool operator!=(const pgm_accessor& compare_to) const
    {
#if FEATURE_ESTD_PGM_EXP_IT
        if(!(is_null() && compare_to.is_null())) return true;
#endif
        return p != compare_to.p;
    }
};


}

namespace internal { namespace impl {

template <class T, bool near>
struct pgm_accessor_impl
{
    ESTD_CPP_STD_VALUE_TYPE(T);

private:
    const_pointer p;

public:
    constexpr explicit pgm_accessor_impl(const_pointer p) : p{p}   {}

    template <class Alloc>
    constexpr explicit pgm_accessor_impl(Alloc, const_pointer p) : p{p}       {}

    typedef const_pointer& offset_type;
    typedef const const_pointer& const_offset_type;
    typedef value_type locked_type;
    typedef value_type const_locked_type;

    offset_type offset() { return p; }
    const_offset_type offset() const { return p; }

    locked_type lock() { return pgm_read<T, near>(p); }
    const_locked_type lock() const { return pgm_read<T, near>(p); }
    static void unlock() {}
};

template <class T, bool near = true>
using pgm_accessor2 = estd::internal::locking_accessor<pgm_accessor_impl<T, near> >;

}}

}