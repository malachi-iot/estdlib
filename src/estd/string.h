#pragma once

#include "memory.h"

namespace estd {

template<class CharT> struct char_traits;

template<> struct char_traits<char>
{
    char char_type;
    int int_type;
};

// TODO: Determine how to organize different string implementations
// a) wrapper around standard C null terminated variety
// b) wrapper around pascal-style length tracking variety (which we'll also combine with dynamic allocation)
// thinking a) would be a good layer2 string and b) would be a good layer3
template<
    class CharT,
    class Traits = char_traits<CharT>,
    class Allocator = nothing_allocator
> class basic_string :
        public experimental::dynamic_array<CharT, Allocator>
{
    typedef experimental::dynamic_array<CharT, Allocator> base_t;
public:
    typedef CharT value_type;
    typedef Traits traits_type;
    typedef Allocator allocator_type;

    typedef typename allocator_type::handle_type handle_type;

    allocator_type allocator;

    basic_string& append(const value_type* s)
    {
        size_t len = strlen(s);

        base_t::_append(s, len);

        return *this;
    }

    template <class TString>
    basic_string& operator += (TString s)
    {
        return append(s);
    }
};


typedef basic_string<char> string;

}
