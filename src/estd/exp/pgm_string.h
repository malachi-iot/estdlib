#include <avr/pgmspace.h>

#include <estd/internal/dynamic_array.h>
#include "../string.h"

namespace estd {

namespace internal {

namespace impl {

struct pgm_allocator
{
    using value_type = char;
    using pointer = const PROGMEM char*;
    using handle_type = pointer;
};


struct pgm_allocator_traits
{
    using value_type = char;
    using pointer = const PROGMEM char*;
    using const_pointer = pointer;
    using handle_type = pointer;
    using handle_with_offset = pointer;
    using size_type = size_t;

    static CONSTEXPR bool is_stateful_exp = false;
    static CONSTEXPR bool is_locking_exp = false;
};

struct PgmPolicy : pgm_allocator_traits
{
    using char_traits = estd::char_traits<char>;
};


// TODO: Phase this guy out
template <class TAllocator>
struct dynamic_array<TAllocator, PgmPolicy>
{
    using size_type = size_t;

    using allocator_type = pgm_allocator;
    using allocator_traits = pgm_allocator_traits;

    using value_type = allocator_type::value_type;

    // FIX: This is incorrect, we'll need a full iterator to do pgm_read_byte
    using iterator = char*;
    using accessor = iterator;

    static CONSTEXPR bool uses_termination() { return false; }

    dynamic_array(const char* const) {}

    // DEBT: Feed this via allocator
    static constexpr size_type size() { return 10; }

    template <class TAllocator2, class TPolicy>
    void append_to_other(dynamic_array<TAllocator2, TPolicy>& other)
    {
    }
};

}

// TODO: Phase this guy out
template <>
struct allocated_array<impl::dynamic_array<impl::pgm_allocator, impl::PgmPolicy> > :
    impl::dynamic_array<impl::pgm_allocator, impl::PgmPolicy> 
{
    using base_type = impl::dynamic_array<impl::pgm_allocator, impl::PgmPolicy>;
    using impl_type = impl::dynamic_array<impl::pgm_allocator, impl::PgmPolicy>;

    allocated_array(const char* const s) : base_type(s) {}
};

template <>
struct basic_string<impl::pgm_allocator, impl::PgmPolicy> : impl::PgmPolicy
{
    using allocator_type = impl::pgm_allocator;
    using allocator_traits = impl::pgm_allocator_traits;

    basic_string(const char* const s) {}
};


}

template <>
struct allocator_traits<internal::impl::pgm_allocator> :
    internal::impl::pgm_allocator_traits
{

};




//template <size_t N>
//using pgm_string = layer2::basic_string<char, N, false, char_traits<char>, internal::impl::PgmPolicy>;

/*
using pgm_string = basic_string<char, char_traits<char>,
    internal::impl::pgm_allocator,
    internal::impl::PgmPolicy>; */

struct pgm_string : basic_string<char, estd::char_traits<char>,
    internal::impl::pgm_allocator,
    internal::impl::PgmPolicy>
{
    using base_type = basic_string<char, estd::char_traits<char>,
        internal::impl::pgm_allocator,
        internal::impl::PgmPolicy>;

    pgm_string(const char* const s) : base_type(s) {}
};

}

