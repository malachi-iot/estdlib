#include "../fwd/string_view.h"

#pragma once

namespace estd { namespace layer1 {

template<class CharT, size_t N, bool null_terminated, class Traits,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy>
class basic_string
        : public estd::basic_string<
                CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                // DEBT: underlying conditional_t deduces CharT[N], just taking extra precautions in
                // light of https://github.com/malachi-iot/estdlib/issues/88
                estd::layer1::allocator<CharT, N, CharT[N]>,
#else
                estd::internal::single_fixedbuf_allocator <CharT, N>,
#endif
                StringPolicy>
{
    typedef estd::basic_string<
                CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer1::allocator<CharT, N>,
#else
                estd::internal::single_fixedbuf_allocator <CharT, N>,
#endif
                StringPolicy>
                base_type;

    typedef base_type base_t;

public:
    basic_string() = default;

    basic_string(const CharT* s)
    {
        base_t::operator =(s);
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

    // layer1 strings can safely issue a lock like this, since unlock is a no-op
    CharT* data() { return base_t::lock(); }

    CharT* c_str()
    {
#if __cpp_static_assert
        static_assert(null_terminated, "Only works for null terminated strings");
#endif
        return data();
    }

    ESTD_CPP_CONSTEXPR_RET const CharT* data() const { return base_t::clock(); }

    const CharT* c_str() const
    {
#if __cpp_static_assert
        static_assert(null_terminated, "Only works for null terminated strings");
#endif
        return data();
    }


    // A little clumsy since basic_string_view treats everything as const already,
    // so if we are converting from a const_string we have to remove const from CharT
    typedef basic_string_view<CharT, Traits> view_type;

    ESTD_CPP_CONSTEXPR_RET operator view_type() const
    {
        return view_type(data(), base_t::size());
    }
};



}}


