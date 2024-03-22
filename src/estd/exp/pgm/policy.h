#pragma once

#include <estd/internal/fwd/variant.h>

namespace estd {

namespace internal {

namespace impl {

template <size_t N>
struct PgmPolicy<char, PgmPolicyType::String, N>
{
    using allocator_traits = pgm_allocator_traits<char, N>;
    using allocator_type = typename allocator_traits::allocator_type;
    using char_traits = estd::char_traits<const char>;

    //static constexpr size_t size() { return N; }

    // DEBT: Align this nomenclature with how 'span' does it,
    // since we're kind of making a fancy span here
    // DEBT: Along those lines, consider making a private_span alongside
    // private_array
    static constexpr bool null_terminated = N == estd::internal::variant_npos();

    // DEBT: May want to specialize further if we think it will optimize more
    static size_t size(const char* data)
    {
        return null_terminated ?
            strnlen_P(data, 256) :
            N;
    }
};


template <class T, size_t N>
struct PgmPolicy<T, PgmPolicyType::BufferInline, N>
{
    // Prefer a has-a to an is-a I think
    // DEBT: As we transition to has-a, we have a doubling up from inheritance also
    using allocator_traits = layer1_pgm_allocator_traits<T, N>;
    using allocator_type = typename allocator_traits::allocator_type;

    static constexpr size_t size(const T*) { return N; }
};


/*
 * TODO: Forgot to consider null term vs fixed size vs runtime size
 * Thinking runtime size just shouldn't be supported for pgm_string,
 * but low level private_array will likely want it eventually
template <>
struct PgmPolicy<estd::internal::variant_npos()> : pgm_allocator_traits
{
    using char_traits = estd::char_traits<char>;
    size_t size_;

    constexpr size_t size() const { return size_; }
    static constexpr bool has_extent = false;
};*/



}   // impl


template <size_t N>
using PgmStringPolicy = impl::PgmPolicy<char, impl::PgmPolicyType::String, N>;


}   // internal

}
