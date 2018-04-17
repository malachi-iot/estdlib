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

    typedef typename base_t::size_type size_type;
    typedef typename allocator_type::handle_type handle_type;

    allocator_type allocator;

    size_type length() const { return base_t::size(); }

    // NOTE: dropping const due to locking operation, but it's debatable whether we
    // want to propagate that const behavior all the way up to here
    size_type copy(value_type* dest, size_type count, size_type pos = 0)
    {
        value_type* src = base_t::lock();

        // TODO: since we aren't gonna throw an exception, determine what to do if
        // pos > size()

        if(pos + count > length())
            count = length() - pos;

        memcpy(dest, src + pos, count);

        base_t::unlock();

        return count;
    }

    basic_string& append(size_type count, value_type c)
    {
        while(count--) base_t::push_back(c);

        return *this;
    }


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


    basic_string& operator += (value_type c)
    {
        push_back(c);
    }


    const value_type front() const
    {
        //value_type* raw = lock();
    }
};


typedef basic_string<char> string;

}
