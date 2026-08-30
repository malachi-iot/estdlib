#pragma once

#include "../fwd/string.h"
#include "../fwd/string_view.h"
#include "../../policy/string.h"
#include "../string.h"

// TODO: refactor layer2 & layer3 to utilize a handle and not a CharT*
namespace estd { namespace layer2 {

// DEBT: Rework to only handle null terminated, because otherwise we're looking
// at a fixed-size string which would be better served by a
// layer1::basic_string_view
template<class Char, size_t N, bool null_terminated,
         class Traits,
         ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy>
class basic_string : public estd::internal::basic_string<
    estd::layer2::allocator<Char, N>,
    StringPolicy >
{
    using base_type = estd::internal::basic_string<
            estd::layer2::allocator<Char, N>,
            StringPolicy>;
    using base_t = base_type;
    typedef typename base_t::impl_type helper_type;

public:
    using typename base_type::view_type;
    using typename base_type::allocator_type;
    using typename base_type::size_type;
    using base_type::data;

    // this one we presume we're looking at either:
    // - an already initialized null terminated string
    // - a size=capacity variant, in which str_buffer isn't (necessarily) null terminated
    //   but size() still reflects the right size of the string
    // This particular constructor is good for string literals, assuming CharT is const char
    constexpr basic_string(Char* str_buffer) : base_type(str_buffer)
    {
    }

    // n means assign length to n, ignoring any null termination if present
    basic_string(Char* str_buffer, int n) : base_type(str_buffer)
    {
        // doing this separately from above constructor because not all
        // specializations permit explicitly (re)sizing the string
        base_type::impl().size(n);
    }

    /// Alternate initializer, explicitly demanding whether to initialize string
    /// @param str_buffer
    /// @param null_terminate_init
    basic_string(Char* str_buffer, bool null_terminate_init) : base_type(str_buffer)
    {
        static_assert(null_terminated, "Constructor only valid for null terminated strings");

        if(null_terminate_init)
            str_buffer[0] = 0;
    }

    template <size_type IncomingN>
    basic_string(Char (&buffer) [IncomingN]) : base_type(&buffer[0])
    {
        static_assert(IncomingN >= N || N == 0, "Incoming buffer size incompatible");
    }

    // See 'n' documentation above
    // FIX: above constructor greedily consumes this one's chance at running.
    // Before, I was using const CharT* to differenciate it but technically
    // a const CharT* is just incorrect as the underlying layer2::basic_string
    // isn't intrinsically const
    template <size_type IncomingN>
    basic_string(Char (&buffer) [IncomingN], int n) : base_type(&buffer[0])
    {
        static_assert(IncomingN >= N || N == 0, "Incoming buffer size incompatible");

        // FIX: for scenarios where:
        // a) C++03/98 is in effect and
        // b) policy size_equals_capacity is in effect,
        // this fails (i.e. assignment to string literal) as compiler attempts to compile/run
        // this even though n == -1
        if(n >= 0) base_t::impl().size(n);
    }

    template <class Impl>
    ESTD_CPP_CONSTEXPR(14) basic_string(estd::internal::allocated_array<Impl>& copy_from)
        // DEBT: only permit this operation when copy_from is a null term string,
        // otherwise copy_from won't know if we've changed the str len.
        : base_type(copy_from.lock())
    {
        copy_from.unlock();
    }

    template <class It>
    basic_string(Char* buffer, It first, It last) : base_type(buffer)       // NOLINT
    {
        // DEBT: Having to brute force 'false' (non shrink, non reallocate) is clunky and inconsistent
        // with above 'assign'
        base_type::assign(first, last, false);
    }

    // Assigns incoming copy_from to whatever pointer we are tracking.
    template <class Impl>
    basic_string& operator=(const estd::internal::allocated_array<Impl>& copy_from) // NOLINT
    {
        base_type::operator =(copy_from);
        return *this;
    }

    basic_string& operator=(const Char* s)
    {
        //return base_t::operator =(s);
        base_t::assign(s, strlen(s));
        return *this;
    }

    Char* c_str()
    {
        static_assert(null_terminated, "Only works for null terminated strings");

        return data();
    }

    constexpr const Char* c_str() const
    {
        static_assert(null_terminated, "Only works for null terminated strings");

        return data();
    }

    constexpr operator typename base_type::view_type() const
    {
        return { data(), base_type::size() };
    }
};


using const_string = basic_string<const char, 0>;
using const_wstring = basic_string<const wchar_t, 0>;

namespace experimental {

template <size_t N>
basic_string<char, N, true> make_string(char (&buffer)[N], int n = -1)
{
    return basic_string<char, N, true>(buffer, n);
}

}

}}
