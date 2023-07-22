#pragma once

#include "fwd/string.h"

#include "dynamic_array.h"

namespace estd { namespace internal {

// DEBT: Get some c++20 concept going for Policy, and also document it
// DEBT: Pretty sure I'd like Allocator wrapped up into policy too
template <class Allocator, class Policy>
class basic_string : public internal::dynamic_array<internal::impl::dynamic_array<Allocator, Policy> >
{
protected:
    typedef internal::dynamic_array<internal::impl::dynamic_array<Allocator, Policy> > base_type;

public:
    typedef Allocator allocator_type;
    typedef typename base_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename Policy::char_traits traits_type;

protected:
    template <class THelperParam>
    EXPLICIT basic_string(const THelperParam& p) : base_type(p) {}

public:
    ESTD_CPP_DEFAULT_CTOR(basic_string)

    template <class TImpl>
    EXPLICIT basic_string(const internal::allocated_array<TImpl>& copy_from) :
        base_type(copy_from) {}
};

}}