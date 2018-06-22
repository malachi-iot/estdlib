#pragma once

#include "../internal/platform.h"

namespace estd {

/*
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
}; */


namespace experimental {


// explicit constant specified here because char_traits by convention doesn't
// specify const
template <class TSize, bool constant>
struct buffer_policy
{
    typedef TSize size_type;

    static CONSTEXPR bool is_constant() { return constant; }
};

template <class TCharTraits, class TSize = int16_t, bool constant = false >
struct string_policy : buffer_policy<TSize, constant>
{
    typedef TCharTraits char_traits;
};


template <class TCharTraits, class TSize = int16_t, bool constant = false>
struct null_terminated_string_policy : public string_policy<TCharTraits, TSize, constant>
{
    static CONSTEXPR bool is_null_terminated() { return true; }

    static CONSTEXPR bool is_null_termination(const char& value) { return value == 0; }
};


template <class TCharTraits, class TSize = int16_t, bool constant = false>
struct sized_string_traits  : public string_policy<TCharTraits, TSize, constant>
{
    static CONSTEXPR bool is_null_terminated() { return false; }
};


}


}
