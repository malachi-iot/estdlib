#pragma once

#include "fwd/string.h"

#include "dynamic_array.h"

namespace estd { namespace internal {

// DEBT: Get some c++20 concept going for Policy & Impl, and also document it
// DEBT: Clean up name - artifact of splitting this out from legacy non-impl flavor.
// if all works out, internal::basic_string2 will become detail::basic_string
template <class Impl>
class basic_string2 : public internal::dynamic_array<Impl>
{
protected:
    typedef internal::dynamic_array<Impl> base_type;

    typedef typename base_type::helper helper;

public:
    typedef typename base_type::allocator_type  allocator_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::impl_type::policy_type policy_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename policy_type::char_traits traits_type;
    typedef typename allocator_type::pointer pointer;
    // DEBT: Get this from someone's traits
    //typedef typename allocator_type::const_pointer const_pointer;
    typedef const value_type* const_pointer;

    static CONSTEXPR size_type npos = (size_type) -1;

protected:
    ESTD_CPP_FORWARDING_CTOR(basic_string2)

    int compare(const_pointer s, size_type s_size) const
    {
        size_type raw_size = base_type::size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        return helper::compare(*this, s, s_size);
    }

public:
    ESTD_CPP_DEFAULT_CTOR(basic_string2)

    size_type length() const { return base_type::size(); }

    template <class Impl2>
    int compare(const internal::allocated_array<Impl2>& a) const
    {
        // NOTE: Underlying compare never pays attention to null termination,
        // so we are safe comparing against a non-string here
        
        int result = compare(a.clock(), a.size());
        a.cunlock();

        return result;
    }

    // compare to a C-style string
    int compare(const_pointer s) const
    {
        return compare(s, strlen(s));
    }


    // compare to a C-style string
    bool starts_with(const_pointer compare_to) const
    {
        return helper::starts_with(*this, compare_to);
    }



    // Keeping this as I expect to eventually need a string/char traits aware
    // version of the character-by-character comparison
    template <class Impl2>
    bool starts_with(const internal::allocated_array<Impl2>& compare_to) const  // NOLINT
    {
        return base_type::starts_with(compare_to);
    }
};

template <class Allocator, class Policy>
class basic_string : public basic_string2<internal::impl::dynamic_array<Allocator, Policy> >
{
protected:
    typedef basic_string2<internal::impl::dynamic_array<Allocator, Policy> > base_type;

public:
    ESTD_CPP_FORWARDING_CTOR(basic_string)
};


}}