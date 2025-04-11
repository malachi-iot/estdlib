#include "../../allocators/fixed.h"
#include "../fwd/string_view.h"

#pragma once

namespace estd { namespace layer1 {

template<class CharT, size_t N, bool null_terminated, class Traits,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy>
class basic_string : public estd::internal::basic_string<
    // DEBT: underlying conditional_t deduces CharT[N], just taking extra precautions in
    // light of https://github.com/malachi-iot/estdlib/issues/88
    estd::layer1::allocator<CharT, N, CharT[N]>,
    StringPolicy>
{
    using base_type = estd::internal::basic_string<
        estd::layer1::allocator<CharT, N>,
        StringPolicy>;

public:
    using typename base_type::view_type;
    using base_type::data;

    basic_string() = default;

    basic_string(const CharT* s)
    {
        base_type::operator =(s);
    }

    template <class Impl>
    basic_string(const estd::internal::allocated_array<Impl>& copy_from)
    {
        base_type::operator=(copy_from);
    }

    template <class Impl>
    basic_string& operator=(const estd::internal::allocated_array<Impl>& copy_from)   // NOLINT
    {
        base_type::operator=(copy_from);
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

    constexpr operator view_type() const
    {
        return { data(), base_type::size() };
    }
};



}}


