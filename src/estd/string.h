#pragma once

#include "memory.h"

namespace estd {

template<class CharT> struct char_traits;

template<> struct char_traits<char>
{
    typedef char char_type;
    typedef int int_type;
    typedef char nonconst_char_type;
};


template<> struct char_traits<const char>
{
    typedef const char char_type;
    typedef int int_type;
    typedef char nonconst_char_type;
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

namespace experimental {

template <class CharT, class TBuffer, class Traits = char_traits<CharT>>
class basic_string_base
{
protected:
    TBuffer buffer;

public:
    template <class TBufferParam>
    basic_string_base(TBufferParam in) : buffer(in) {}

    typedef CharT value_type;
    typedef size_t size_type;
};

namespace layer2 {

// represents a null-terminated string with a constexpr-size.  Should be nearly identical to
// a layer3 variety with a non-constexpr-size
// experimental because even though this is quite possible, combining it with regular basic_string
// is both desired but complicated
// TODO: Probably we want to distinguish this as null-terminated vs not null terminated either with
// traits or an explicit name
template <class CharT, size_t N, class Traits = char_traits<CharT>>
class basic_string : public basic_string_base<CharT, CharT*, Traits>
{
protected:
    typedef basic_string_base<CharT, CharT*> base_t;

public:
    typedef CharT value_type;
    typedef size_t size_type;
    typedef typename Traits::nonconst_char_type nonconst_char_type;

    basic_string(CharT* buffer) : base_t(buffer) {}

    size_type size() const
    {
        return strlen(base_t::buffer);
    }

    size_type length() const { return size(); }

    size_type max_size() const { return N - 1; }

    size_type copy(nonconst_char_type* dest, size_type count, size_type pos = 0)
    {
        // TODO: Do bounds checking etc.
        strncpy(dest, base_t::buffer + pos, count);
        return count;
    }
};

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N>
using string = basic_string<char, N>;
#endif


}


namespace layer3 {

// represents a null-terminated string with a particular specified size
// TODO: Probably we want to distinguish this as null-terminated vs not null terminated either with
// traits or an explicit name
template <class CharT, class Traits = char_traits<CharT>>
class basic_string : public basic_string_base<CharT, CharT*, Traits>
{
public:
    typedef size_t size_type;

protected:
    size_type m_length;
    typedef basic_string_base<CharT, CharT*> base_t;

public:
    typedef CharT value_type;
    typedef typename Traits::nonconst_char_type nonconst_char_type;

    template <size_t N>
    basic_string(CharT (&buffer) [N]) :
        base_t(buffer), m_length(N) {}

    basic_string(CharT* buffer, size_type length) :
        base_t(buffer), m_length(length)
    {}

    size_type size() const
    {
        return strlen(base_t::buffer);
    }

    size_type length() const { return size(); }

    size_type max_size() const { return m_length - 1; }

    size_type copy(nonconst_char_type* dest, size_type count, size_type pos = 0)
    {
        // TODO: Do bounds checking etc.
        strncpy(dest, base_t::buffer + pos, count);
        return count;
    }
};
}

}

}
