#pragma once

#include "../fwd/string_view.h"
#include "../../policy/string.h"

// TODO: refactor layer2 & layer3 to utilize a handle and not a CharT*
namespace estd { namespace layer2 {

// DEBT: Rework to only handle null terminated, because otherwise we're looking
// at a fixed-size string which would be better served by a
// layer1::basic_string_view
template<class CharT, size_t N, bool null_terminated,
         class Traits,
         ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy>
class basic_string : public estd::internal::basic_string<
    estd::layer2::allocator<CharT, N>,
    StringPolicy >
{
    using base_type = estd::internal::basic_string<
            estd::layer2::allocator<CharT, N>,
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
    constexpr basic_string(CharT* str_buffer) : base_type(str_buffer)
    {
    }

    // n means assign length to n, ignoring any null termination if present
    basic_string(CharT* str_buffer, int n) : base_t(str_buffer)
    {
        // doing this separately from above constructor because not all
        // specializations permit explicitly (re)sizing the string
        base_t::impl().size(n);
    }

    /// Alternate initializer, explicitly demanding whether to initialize string
    /// \param str_buffer
    /// \param null_terminate_init
    basic_string(CharT* str_buffer, bool null_terminate_init) : base_t(str_buffer)
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert(null_terminated, "Constructor only valid for null terminated strings");
#endif

        if(null_terminate_init)
            str_buffer[0] = 0;
    }

    template <size_type IncomingN>
    basic_string(CharT (&buffer) [IncomingN]) : base_t(&buffer[0])
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert(IncomingN >= N || N == 0, "Incoming buffer size incompatible");
#endif
    }

    // See 'n' documentation above
    // FIX: above constructor greedily consumes this one's chance at running.
    // Before, I was using const CharT* to differenciate it but technically
    // a const CharT* is just incorrect as the underlying layer2::basic_string
    // isn't intrinsically const
    template <size_type IncomingN>
    basic_string(CharT (&buffer) [IncomingN], int n) : base_t(&buffer[0])
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert(IncomingN >= N || N == 0, "Incoming buffer size incompatible");
#endif

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

    // Assigns incoming copy_from to whatever pointer we are tracking.
    template <class Impl>
    basic_string& operator=(const estd::internal::allocated_array<Impl>& copy_from) // NOLINT
    {
        base_type::operator =(copy_from);
        return *this;
    }

    basic_string& operator=(const CharT* s)
    {
        //return base_t::operator =(s);
        base_t::assign(s, strlen(s));
        return *this;
    }

    CharT* c_str()
    {
#if __cpp_static_assert
        static_assert(null_terminated, "Only works for null terminated strings");
#endif
        return data();
    }

    constexpr const CharT* c_str() const
    {
#if __cpp_static_assert
        static_assert(null_terminated, "Only works for null terminated strings");
#endif
        return data();
    }

    constexpr operator typename base_type::view_type() const
    {
        return { data(), base_type::size() };
    }
};


#ifdef __cpp_alias_templates
template <size_t N = 0, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;
#endif

typedef basic_string<const char, 0> const_string;

namespace experimental {

template <size_t N>
basic_string<char, N, true> make_string(char (&buffer)[N], int n = -1)
{
    return basic_string<char, N, true>(buffer, n);
}

}

}}
