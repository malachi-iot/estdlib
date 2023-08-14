#pragma once

#include "../fwd/string_view.h"
#include "../../policy/string.h"

// TODO: refactor layer2 & layer3 to utilize a handle and not a CharT*
namespace estd { namespace layer2 {

template<class CharT, size_t N, bool null_terminated = true,
         class Traits = estd::char_traits<typename estd::remove_const<CharT>::type >,
         class StringPolicy = typename estd::conditional<null_terminated,
                estd::experimental::null_terminated_string_policy<Traits, int16_t, estd::is_const<CharT>::value>,
                estd::experimental::sized_string_policy<Traits, int16_t, estd::is_const<CharT>::value> >::type >
class basic_string
        : public estd::basic_string<
                CharT,
                Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer2::allocator<CharT, N>,
#else
                estd::internal::single_fixedbuf_allocator < CharT, N, CharT* >,
#endif
                StringPolicy >
{
    typedef estd::basic_string<
            CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
            estd::layer2::allocator<CharT, N>,
#else
            estd::internal::single_fixedbuf_allocator < CharT, N, CharT* >,
#endif
            StringPolicy >
            base_type;
    typedef base_type base_t;
    typedef typename base_t::impl_type helper_type;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::size_type size_type;

    // this one we presume we're looking at either:
    // - an already initialized null terminated string
    // - a size=capacity variant, in which str_buffer isn't (necessarily) null terminated
    //   but size() still reflects the right size of the string
    // This particular constructor is good for string literals, assuming CharT is const char
    basic_string(CharT* str_buffer) : base_t(str_buffer)
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
    basic_string(estd::internal::allocated_array<Impl>& copy_from)
        // FIX: very bad -- don't leave things locked!  Also, we really want
        // to only permit this operation when copy_from is a null term string,
        // otherwise copy_from won't know if we've changed the str len.
        // Only doing this because we often pass around layer1, layer2, layer3 strings who
        // don't care about lock/unlock
        : base_type(copy_from.lock())
    {
    }

    // DEBT: Similar to above constructor, we probably want to filter this
    // by compatible resizing scenarios
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

    // layer2 strings can safely issue a lock like this, since unlock is a no-op
    CharT* data() { return base_t::lock(); }

    const CharT* data() const { return base_t::clock(); }

    // A little clumsy since basic_string_view treats everything as const already,
    // so if we are converting from a const_string we have to remove const from CharT
    typedef basic_string_view<typename estd::remove_const<CharT>::type, Traits> view_type;

    ESTD_CPP_CONSTEXPR_RET operator view_type() const
    {
        return view_type(data(), base_t::size());
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
