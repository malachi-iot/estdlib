/**
 * @file
 * NOTE: data() implementation sticks to <= c++11 form in which does not need to be null terminated
 */
#pragma once

#include "memory.h"
#include "allocators/fixed.h"
#include "traits/char_traits.h"
#include "traits/string.h"
#include "internal/string.h"
#include "internal/fwd/string_view.h"
#include "policy/string.h"
#include "port/string.h"
#include "algorithm.h"
#include "span.h"
#include "cstdlib.h"

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
#include <ostream>
#endif

#include "internal/macro/push.h"

namespace estd {

// TODO: Determine how to organize different string implementations
// a) wrapper around standard C null terminated variety
// b) wrapper around pascal-style length tracking variety (which we'll also combine with dynamic allocation)
// thinking a) would be a good layer2 string and b) would be a good layer3
// We can start switching 'layer' version of string to derive from basic_string itself by using
// fixed allocators
// TODO: Document why it's necessary to remove_const for char_traits CharT
///
/// @tparam CharT
/// @tparam Traits
/// @tparam Allocator
/// @tparam StringPolicy
/// @remarks Largely a wrapper around internal::basic_string, which is much less clumsy
template<
    class CharT,
    class Traits = estd::char_traits<typename estd::remove_const<CharT>::type >,
#ifdef FEATURE_STD_MEMORY
    class Allocator = std::allocator<CharT>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy = experimental::sized_string_policy<Traits>
#else
    class Allocator, ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy
#endif
> class basic_string : public internal::basic_string<Allocator, StringPolicy>
{
    typedef internal::basic_string<Allocator, StringPolicy> base_t;
    typedef base_t base_type;

    /*
     * Was using this for Arduino port += but actually it's not necessary and causes linker
     * errors for POSIX mode
    template <class TInput>
    friend basic_string operator +=(basic_string& lhs, TInput rhs); */

public:
    typedef typename base_type::size_type size_type;

    // DEBT: At the moment more a compiler helper so that stringbuf can compile, even if
    // its view() isn't called
    typedef basic_string_view<CharT, Traits> view_type;

protected:
    ESTD_CPP_FORWARDING_CTOR(basic_string)

    // DEBT: Does not conform to 'strong exception safety guarantee'
    static void assert_append(typename base_type::append_result r)
    {
#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        if(r.has_value() == false)
        {
#if __cpp_exceptions
            throw std::length_error("Could not allocate enough memory");
#else
            std::abort();
#endif
        }
#endif
    }

public:
    ESTD_CPP_DEFAULT_CTOR(basic_string)

    template <class TImpl>
    basic_string(const internal::allocated_array<TImpl>& copy_from) :
        base_type(copy_from) {}

    typedef CharT value_type;
    typedef Traits traits_type;
    typedef typename base_t::allocator_type allocator_type;
    typedef StringPolicy policy_type;
    typedef typename base_type::const_pointer const_pointer;

    typedef typename allocator_type::handle_type handle_type;

    static CONSTEXPR size_type npos = base_type::npos;

    basic_string& erase(size_type index = 0, size_type count = npos)
    {
        size_type size_minus_index = base_type::size() - index;
        // NOTE: A bit tricky, if we don't use helper size_minus_index, template
        // resolution fails, presumably because the math operation implicitly
        // creates an int
        size_type to_remove_count = estd::min(count, size_minus_index);

        base_type::_erase(index, to_remove_count);

        return *this;
    }

    basic_string& append(size_type count, value_type c) // NOLINT
    {
        while(count--) *this += c;

        return *this;
    }


    template <class Impl2>
    basic_string& append(const internal::allocated_array<Impl2>& str)
    {
        assert_append(base_type::append(str));
        return *this;
    }

    basic_string& append(const_pointer s, size_type count)  // NOLINT
    {
        assert_append(base_type::append(s, count));
        return *this;
    }


    basic_string& append(const_pointer s)
    {
        return append(s, strlen(s));
    }


    /*
    template <class TString>
    basic_string& operator += (TString s)
    {
        return append(s);
        return *this;
    } */


    basic_string& operator+=(value_type c)
    {
        assert_append(base_type::push_back(c));
        return *this;
    }


    template <class TForeignImpl>
    basic_string& operator=(const experimental::private_array<TForeignImpl>& copy_from)   // NOLINT
    {
        // DEBT: Sloppy
        base_t::base_type::operator =(copy_from);
        return *this;
    }


    template <class TForeignImpl>
    basic_string& operator=(const internal::allocated_array<TForeignImpl>& copy_from)   // NOLINT
    {
        base_type::assign(copy_from);
        return *this;
    }

    basic_string& operator=(const_pointer s)
    {
        base_type::assign(s, strlen(s));
        return *this;
    }
};


// this typedef relies on std::allocator
#ifdef FEATURE_STD_MEMORY
typedef basic_string<char> string;
#endif

// DEBT: Move this out to layer3/string.h if we plan to keep const_string.
// That is difficult because it's template-instantiating basic_string
namespace layer3 {

// Non-NULL-terminated const strings use layer3
// NULL-terminated const strings use layer2
// NOTE: Remake this into basic_string_view.  Though the basic_string_view has an additional variable (potentially)
//       for tracking the truncations
// If the above does not happen, then consider promoting/typedefing this const_string to the regular
// estd:: namespace too
class const_string : public basic_string<const char, false>
{
    typedef basic_string<const char, false> base_t;
    typedef base_t::size_type size_type;

public:
    const_string(const char* s, size_type len) :
        base_t(s, len, true) {}


    template <size_type N>
    const_string(const char (&buffer) [N], bool source_null_terminated = true) :
        base_t(buffer, source_null_terminated ? strlen(buffer) : N, true) {}

    // convenience method since std::vector and std::string itself are reported to convert
    // uneventfully between unsigned char and char
    // note it's a little bit bad because of the distant possibility of a
    // byte != unsigned char != uint8_t
    const_string(const estd::span<const uint8_t>& cast_from) :
        base_t(reinterpret_cast<const char*>(cast_from.data()), cast_from.size(), true) {}
};

}



template< class CharT, class Traits, class Alloc, class Policy, class TString >
    estd::basic_string<CharT,Traits,Alloc,Policy>&
        operator+=(estd::basic_string<CharT,Traits,Alloc,Policy>& lhs,
                   TString rhs )
{
    lhs.append(rhs);
    return lhs;
}


template <class Impl, class T>
detail::basic_string<Impl>& operator+=(detail::basic_string<Impl>& lhs, T rhs)
{
    lhs.append(rhs);
    return lhs;
}



template <class CharT, class Traits, class Alloc, class TStringTraits>
bool operator ==( const CharT* lhs, const basic_string<CharT, Traits, Alloc, TStringTraits>& rhs)
{
    return rhs.compare(lhs) == 0;
}

/*
template <class Traits, class Alloc>
bool operator ==( const basic_string<typename Traits::char_type, Traits, Alloc>& lhs,
                  const typename Traits::char_type* rhs)
{
    return lhs.compare(rhs) == 0;
}


template <class TraitsLeft,
          class TraitsRight,
          class AllocLeft, class AllocRight>
bool operator ==( const basic_string<typename TraitsLeft::char_type, TraitsLeft, AllocLeft>& lhs,
                  const basic_string<typename TraitsRight::char_type, TraitsRight, AllocRight>& rhs)
{
    return lhs.compare(rhs) == 0;
} */

template <class Impl>
ESTD_CPP_CONSTEXPR_RET bool operator ==(
    const detail::basic_string<Impl>& lhs,
    typename detail::basic_string<Impl>::const_pointer rhs)
{
    return lhs.compare(rhs) == 0;
}



// have to do this because of our special const char stuff
// initial tests doing std::char_traits<const char> seem to work, but I don't
// trust them - where do they specialize to?
template <class TCharLeft,
          class TCharRight,
          class StringTraitsLeft,
          class StringTraitsRight,
          class AllocLeft, class AllocRight>
bool operator ==( const basic_string<TCharLeft, typename StringTraitsLeft::char_traits, AllocLeft, StringTraitsLeft>& lhs,
                  const basic_string<TCharRight, typename StringTraitsRight::char_traits, AllocRight, StringTraitsRight>& rhs)
{
    return lhs.compare(rhs) == 0;
}


// FIX: This doesn't account for conversion errors, but should.  std version
// throws exceptions
// NOTE: Retains TChar rather than deducing from Traits because Traits always
// removes const, while TChar can be const.  Will create complexities if and when
// wchar needs to be supported
// DEBT: Rework to use num_get/from_string
template <class TChar, class Traits, class Alloc, class TStringTraits>
long stol(
        const basic_string<TChar, Traits, Alloc, TStringTraits>& str,
        size_t* pos = 0, int base = 10)
{
    // FIX: very clunky way to ensure we're looking at a null terminated string
    // somehow I never expose null_termination indicators on the string itself
    TStringTraits::is_null_termination(0);

    const TChar* data = str.clock();
    typename estd::remove_const<TChar>::type* end;
    long result = strtol(data, &end, base);
    str.cunlock();
    if(pos != NULLPTR)
        *pos = end - data;

    return result;
}

template <class TChar, class Traits, class Alloc, class TStringTraits>
unsigned long stoul(
        const basic_string<TChar, Traits, Alloc, TStringTraits>& str,
        size_t* pos = 0, int base = 10
        )
{
    // FIX: very clunky way to ensure we're looking at a null terminated string
    // somehow I never expose null_termination indicators on the string itself
    TStringTraits::is_null_termination(0);

    const TChar* data = str.clock();
    typename estd::remove_const<TChar>::type* end;
    unsigned long result = strtoul(data, &end, base);
    str.cunlock();
    if(pos != NULLPTR)
        *pos = end - data;

    return result;
}


// TODO: Utilized optimized version for int precision only
// NOTE: Interestingly, spec calls for stoi to call strtol or wcstol.
// TODO: This is  an optimization opportunity, to call something like atoi
// for lower-precision conversion
template <class TChar, class Traits, class Alloc, class TStringTraits>
int stoi(
        const basic_string<TChar, Traits, Alloc, TStringTraits>& str,
        size_t* pos = 0, int base = 10)
{
    return (int) stol(str, pos, base);
}


}


#include "internal/to_string.h"


#include "internal/macro/pop.h"




