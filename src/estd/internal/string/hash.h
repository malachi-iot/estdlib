#pragma once

#include "../fwd/string.h"
#include "../fwd/string_view.h"
#include "../fwd/functional.h"
#include "../functional/hash.h"

namespace estd {

namespace internal {

struct string_hash
{
private:
    // 18AUG26 MB DEBT: We have a number of issues here:
    // 1. begin/end may not be optimized yet for null terminated strings, demanding a strlen when none is needed
    //    See https://github.com/malachi-iot/estdlib/issues/232
    // 2. similar story for .size() call
    // 3. theoretically .data() may not be available (locking memory for example)
    using hasher = fnv_hash<uint32_t>;

    // 8-bit mode
    template <class String>
    static ESTD_CPP_CONSTEXPR(14) uint32_t hash(const String& v, true_type)
    {
        return container_hash<uint32_t>{}(v);
    }

    // 16+ bit mode
    template <class String>
    static ESTD_CPP_CONSTEXPR(14) uint32_t hash(const String& v, false_type)
    {
        using char_type = typename String::value_type;
        const char_type* data = v.data();
        return hasher::hash(
            reinterpret_cast<const uint8_t*>(data),
            reinterpret_cast<const uint8_t*>(data + v.size()));
    }

public:
    template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) StringImpl>
    ESTD_CPP_CONSTEXPR(14) size_t operator()(const detail::basic_string<StringImpl>& v) const
    {
        using char_type = typename StringImpl::value_type;

        return hash(v, bool_constant<sizeof(char_type) == 1>{});
    }
};

}

// 18AUG26 MB DEBT: 16-bit or wider strings aren't going to work with current container_hash

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
