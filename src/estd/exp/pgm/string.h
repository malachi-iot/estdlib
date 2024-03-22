#pragma once

#include "allocator.h"
#include "impl.h"
#include "read.h"
#include "policy.h"

#include "../../internal/string.h"
#include "../../ostream.h"

#include "../../internal/macro/push.h"

namespace estd {

namespace internal {

template <typename T, unsigned N>
struct dynamic_array_helper<avr::impl::pgm_string<T, N> >
{
    typedef avr::impl::pgm_string<T, N> impl_type;

    typedef internal::dynamic_array<impl_type> dynamic_array;
    typedef internal::allocated_array<impl_type> array;

    typedef typename array::value_type value_type;
    typedef typename array::pointer pointer;
    typedef typename array::const_pointer const_pointer;
    typedef typename array::size_type size_type;

    // copy from us to outside dest/other
    static size_type copy_to(const array& a,
        typename estd::remove_const<value_type>::type* dest,
        size_type count, size_type pos = 0)
    {
        const size_type _end = estd::min(count, a.size());
        // DEBT: Not 100% convinced that resolves
        // back to a pointer smoothly or at all, even though compiles OK
        //const_pointer src = a.ofset(pos);
        
        // DEBT: A tad TOO knowledgable about allocated_array internals
        const_pointer src = a.m_impl.data(pos);
        memcpy_P(dest, src, _end * sizeof(T));
        return _end;
    }


    static int compare(const detail::basic_string<impl_type>& lhs, const_pointer rhs, size_type sz)
    {
        const auto& allocator = lhs.get_allocator();
        const_pointer src = allocator.data();

        // NOTE: strcmp_P expects *second* parameter as pgm space,
        // while basic_string operations overall expect basic_string to be
        // lhs - so we have to flip the result here
        return -strncmp_P(rhs, src, sz);
    }

    // rhs = null terminated C string
    // UNTESTED
    template <class InputIt>
    static bool starts_with(const array& lhs, InputIt rhs)
    {
        const_pointer s = lhs.begin();

        // Couldn't find any pgmspace API that really handled this, so doing
        // it manually
        return starts_with(lhs, rhs);
    }
};

// DEBT: We have to manually specialize this guy too because we DON'T
// expose locking_preference at allocator_traits level
template <class O, unsigned N>
struct out_string_helper<O, avr::impl::pgm_string<char, N> >
{

    template <class A>
    static void out(O& out, const A& str)
    {
        out_string_helper_iterated(out, str);
    }
};


#ifdef ARDUINO
template <unsigned N>
struct out_string_helper<arduino_ostream, avr::impl::pgm_string<char, N> >
{
    template <class A>
    static void out(arduino_ostream& out, const A& str)
    {
        const auto& allocator = str.get_allocator();
        const char* data = allocator.data();
        out << reinterpret_cast<const __FlashStringHelper*>(data);
    }
};
#endif


}   // estd::internal

inline namespace v0 { inline namespace avr {

template <class CharT, size_t N = internal::variant_npos()>
struct basic_pgm_string :
    detail::basic_string<avr::impl::pgm_string<CharT, N> >
{
    using base_type = detail::basic_string<avr::impl::pgm_string<CharT, N> >;

    constexpr basic_pgm_string(const char* const s) :
        base_type(s)
    {}
};

using pgm_string = basic_pgm_string<char>;

}}

}

#include "../../internal/macro/pop.h"
