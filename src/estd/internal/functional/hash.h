#pragma once

#include "../fwd/functional.h"

namespace estd {

namespace internal {

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// https://en.cppreference.com/w/cpp/utility/hash/operator()

// NOT READY YET

// Adapted from:
// https://github.com/fabiogaluppo/fnv/blob/main/fnv64.hpp
template <class Precision>
struct fnv_hash;

template <>
struct fnv_hash<uint64_t>
{
    static constexpr uint64_t FNV1_64_INIT = 0xcbf29ce484222325ULL;
    static constexpr uint64_t FNV1A_64_INIT = FNV1_64_INIT;

    static constexpr uint64_t shift_sum(uint64_t hashval)
    {
        return (hashval << 1) + (hashval << 4) + (hashval << 5) +
               (hashval << 7) + (hashval << 8) + (hashval << 40);
    }

    template <class It>
    uint64_t hash(It begin, It end, uint64_t hashval = FNV1_64_INIT)
    {
        It current = begin;
        for (; current != end; ++current)
        {
            hashval += shift_sum(hashval);
            hashval ^= static_cast<std::uint64_t>(*current);
        }
        return hashval;
    }
};

template <>
struct fnv_hash<uint32_t>
{
    // DEBT: Just grabbed this guy out of the air
    static constexpr uint32_t FNV1_32_INIT = 0xcbf29ce4;

    static constexpr uint32_t shift_sum(uint32_t hashval)
    {
        return (hashval << 1) + (hashval << 4) + (hashval << 5) +
               (hashval << 7) + (hashval << 8) + (hashval << 24);
    }

    template <class It>
    uint32_t hash(It begin, It end, uint64_t hashval = FNV1_32_INIT)
    {
        It current = begin;
        for (; current != end; ++current)
        {
            hashval += shift_sum(hashval);
            hashval ^= static_cast<std::uint32_t>(*current);
        }
        return hashval;
    }
};

}

// NOTE: Somewhat experimental, just whipping something up

template <>
struct hash<int>
{
    using precision = unsigned;

    constexpr precision operator()(const int& v) const
    {
        return (precision) v;
    }
};

}
