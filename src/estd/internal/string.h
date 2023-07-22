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
    typedef typename allocator_type::pointer pointer;
    // DEBT: Get this from someone's traits
    //typedef typename allocator_type::const_pointer const_pointer;
    typedef const value_type* const_pointer;

protected:
    ESTD_CPP_FORWARDING_CTOR(basic_string)

public:
    ESTD_CPP_DEFAULT_CTOR(basic_string)

    template <class TImpl>
    EXPLICIT basic_string(const internal::allocated_array<TImpl>& copy_from) :
        base_type(copy_from) {}

    size_type length() const { return base_type::size(); }

    template <class TImpl>
    int compare(const internal::dynamic_array<TImpl>& str) const
    {
        size_type raw_size = base_type::size();
        size_type s_size = str.size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const_pointer raw = base_type::clock();
        const_pointer s = str.clock();

        int result = traits_type::compare(raw, s, raw_size);

        base_type::cunlock();

        str.cunlock();

        return result;
    }

    int compare(const_pointer s) const
    {
        size_type raw_size = base_type::size();
        size_type s_size = strlen(s);

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const_pointer raw = base_type::clock();

        int result = traits_type::compare(raw, s, raw_size);

        base_type::cunlock();

        return result;

    }


    // compare to a C-style string
    bool starts_with(const_pointer compare_to) const
    {
        const value_type* s = base_type::clock();

        size_type source_max = length();

        while(source_max-- && *compare_to != 0)
            if(*s++ != *compare_to++)
            {
                base_type::cunlock();
                return false;
            }

        base_type::cunlock();
        // if compare_to is longer than we are, then it's also a fail
        return source_max != -1;
    }



    // Keeping this as I expect to eventually need a string/char traits aware
    // version of the character-by-character comparison
    template <class TImpl>
    bool starts_with(const internal::allocated_array<TImpl>& compare_to) const
    {
        return base_type::starts_with(compare_to);
    }
};

}}