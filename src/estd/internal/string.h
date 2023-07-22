#pragma once

#include "fwd/string.h"

#include "dynamic_array.h"

namespace estd { namespace internal {

// Compare two strings, assumes rhs is null terminated
template <typename It1, typename It2>
bool starts_with_n(It1 lhs, It2 rhs, int count)
{
    // DEBT: Using signed int here is fiddly, since incoming count likely won't be signed

    while(*rhs != 0 && count--)
        if(*lhs++ != *rhs++)
        {
            return false;
        }

    // if compare_to is longer than we are, then it's also a fail
    return count != -1;
}

// Both are null-terminated here
// DEBT: Make bounded version
template <typename It1, typename It2>
bool starts_with(It1 lhs, It2 rhs)
{
    for(;;)
    {
        // Reaching the end of compare_to string = succeeded
        if(*rhs == 0)   return true;

        // Reaching the end of original string after not reaching compare_to = failed
        // through entertaining magic, this line is automatically handled below since
        // we know rhs now cannot be 0, meaning that if lhs IS zero it will never match rhs,
        // thus returning false
        //else if(*lhs == 0)   return false;

        // While still searching, if we get mismatched characters, we're done
        else if(*lhs++ != *rhs++)    return false;
    }
}


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
        const_pointer s = base_type::clock();

        bool r = starts_with_n(s, compare_to, length());

        base_type::cunlock();
        return r;
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