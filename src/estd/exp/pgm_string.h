#pragma once

#include <avr/pgmspace.h>

#include "pgm/allocator.h"
#include "pgm/impl.h"
#include "pgm/read.h"
#include "pgm/policy.h"

#include <estd/internal/fwd/variant.h>
#include <estd/internal/dynamic_array.h>
#include <estd/internal/container/locking_accessor.h>
#include <estd/internal/container/iterator.h>
#include "../string.h"
#include <estd/ostream.h>

#include "../internal/macro/push.h"

namespace estd {

namespace experimental {

// Null terminated PGM space string core, now considered "legacy" flavor
template <class T, size_t N>
struct pgm_array_string : private_array_base<T, N>
{
    using base_type = private_array_base<T, N>;

    using typename base_type::size_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::value_type;
    using typename base_type::accessor;
    using typename base_type::iterator;

    using const_iterator = iterator;

    constexpr pgm_array_string(const_pointer data) : base_type(data) {}

    constexpr iterator end() const
    {
        return base_type::create_iterator(base_type::size());
    }

    // copies without bounds checking
    void copy_ll(value_type* dest, size_type count, size_type pos = 0) const
    {
#if FEATURE_ESTD_PGM_ALLOCATOR
        iterator source = base_type::create_iterator(pos);
#else
        //iterator source(base_type::data_ + pos);
        iterator source(base_type::data(pos));
#endif

        estd::copy_n(source, count, dest);
    }

    size_type copy(value_type* dest, size_type count, size_type pos = 0) const
    {
        const size_type _end = estd::min(count, base_type::size());
        copy_ll(dest, _end, pos);

        //memcpy_P(dest, base_type::data_ + pos, count);
        
        return _end;
    }


    /* TODO
    template <class TImpl2>
    int compare(const internal::allocated_array<TImpl2>& s) const
    {
        return -1;
    }   */


    int compare(const char* s) const
    {
        if(base_type::null_terminated)
        {
            // FIX: Not fully checked to see if int result aligns with
            // dynamic_array flavor outside of 0/not 0
            return strcmp_P(s, base_type::data());
        }
        else
        {
            // DEBT: do more of a memcmp here
            return -1;
        }
    }

    // Expects regular (non pgm) space string here
    // both compare_to and pgm_string must be null terminated
    bool starts_with(const char* compare_to) const
    {
        //internal::starts_with_n(begin(), compare_to, size());
        return internal::starts_with(base_type::begin(), compare_to);
    }

    // Effectively a shallow copy, since pgm_string largely represents a pointer
    // wrapper
    pgm_array_string& operator=(const pgm_array_string& copy_from)
    {
        base_type::data() = copy_from.data();
        return *this;
    }
};

// specialization on private_array, phasing this out
template<size_t N>
struct private_array<estd::internal::impl::PgmPolicy<char,
    internal::impl::PgmPolicyType::String, N>> :
    pgm_array_string<char, N>
{
    using base_type = pgm_array_string<char, N>;

    constexpr private_array(typename base_type::const_pointer p) :
        base_type(p) {}
};


}

namespace internal {

template <size_t N>
struct basic_string<impl::pgm_allocator, PgmStringPolicy<N>> :
    experimental::private_array<PgmStringPolicy<N>>
{
    using base_type = experimental::private_array<PgmStringPolicy<N>>;
    using allocator_type = impl::pgm_allocator;
    using allocator_traits = impl::pgm_allocator_traits<char>;
    using typename base_type::size_type;

    size_type length() const { return base_type::size(); }

    // Expects PGM space string here
    constexpr basic_string(const char* const s) : base_type(s) {}

#ifdef ARDUINO
    // UNTESTED
    constexpr basic_string(const __FlashStringHelper* s) :
        base_type(reinterpret_cast<const char*>(s))
    {}
#endif
};

/*
template <size_t N>
struct basic_string2<experimental::pgm_array_string<char, N> >
{

};  */



}   // estd::internal


//template <size_t N>
//using pgm_string = layer2::basic_string<char, N, false, char_traits<char>, internal::impl::PgmPolicy>;

/*
using pgm_string = basic_string<char, char_traits<char>,
    internal::impl::pgm_allocator,
    internal::impl::PgmPolicy>; */

template <size_t N = internal::variant_npos()>
struct basic_pgm_string : basic_string<char, estd::char_traits<char>,
    internal::impl::pgm_allocator,
    internal::PgmStringPolicy<N>>
{
    using base_type = basic_string<char, estd::char_traits<char>,
        internal::impl::pgm_allocator,
        internal::PgmStringPolicy<N>>;

    constexpr basic_pgm_string(const char* const s) : base_type(s) {}
};


using pgm_string = basic_pgm_string<>;

// Special case insertion operator for arduino streams who can handle
// PGM space directly.  Considered "legacy" mode now
template <size_t N>
constexpr arduino_ostream& operator <<(arduino_ostream& out,
    const experimental::pgm_array_string<char, N>& s)
{
    return out << reinterpret_cast<const __FlashStringHelper*>(s.data());
}

}

