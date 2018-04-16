#pragma once


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
    class Allocator = std::allocator<CharT>
> class basic_string
{

};


typedef basic_string<char> string;

}
