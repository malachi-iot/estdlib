#pragma once

#include "fwd.h"
#include "read.h"

namespace estd {

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