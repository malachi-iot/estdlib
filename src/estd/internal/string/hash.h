#pragma once

#include "../fwd/string.h"
#include "../fwd/string_view.h"
#include "../fwd/functional.h"
#include "../functional/hash.h"

namespace estd {

namespace internal {

struct string_hash
{
    template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) StringImpl>
    ESTD_CPP_CONSTEXPR(14) size_t operator()(const detail::basic_string<StringImpl>& v) const
    {
        return container_hash<uint32_t>{}(v);
    }
};

}

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) StringImpl>
struct hash<detail::basic_string<StringImpl>> : internal::string_hash {};

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) StringImpl>
struct hash<const detail::basic_string<StringImpl>> : internal::string_hash {};

template <class Char, size_t N, bool null_terminated, class Traits>
struct hash<layer1::basic_string<Char, N, null_terminated, Traits>> : internal::string_hash {};

template <class Char, size_t N, bool null_terminated, class Traits>
struct hash<const layer1::basic_string<Char, N, null_terminated, Traits>> : internal::string_hash {};

template <class Char, size_t N, bool null_terminated, class Traits>
struct hash<layer2::basic_string<Char, N, null_terminated, Traits>> : internal::string_hash {};

template <class Char, size_t N, bool null_terminated, class Traits>
struct hash<const layer2::basic_string<Char, N, null_terminated, Traits>> : internal::string_hash {};

// DEBT: Not well tested and in wrong location
template <class Policy>
struct hash<detail::basic_string_view<Policy>> : internal::string_hash {};

}
