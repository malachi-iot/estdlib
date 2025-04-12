#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../../cstdint.h"

namespace estd {

namespace internal {

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// https://en.cppreference.com/w/cpp/utility/hash/operator()

// While looking for 16-bit variants, found none.  Instead, came across this:
// https://www.reddit.com/r/RNG/comments/jqnq20/the_wang_and_jenkins_integer_hash_functions_just/

enum fnv_modes
{
    FNV_1,
    FNV_1A
};

// Adapted from:
// https://github.com/fabiogaluppo/fnv/blob/main/fnv64.hpp
// https://gist.github.com/hwei/1950649d523afd03285c

// GCC and Clang informally rely on FNV_1A, so so will we
template <class Precision, fnv_modes = FNV_1A>
struct fnv_hash;

struct fnv_hash_base
{
    static constexpr uint64_t FNV1_64_INIT = 0xcbf29ce484222325ULL;
    static constexpr uint64_t FNV1A_64_INIT = FNV1_64_INIT;

    static constexpr uint32_t FNV1A_32_PRIME = 16777619u;
    static constexpr uint32_t FNV1A_32_OFFSET_BASIS = 2166136261u;

    static constexpr uint64_t shift_sum(uint64_t hashval)
    {
        return (hashval << 1) + (hashval << 4) + (hashval << 5) +
            (hashval << 7) + (hashval << 8) + (hashval << 40);
    }
};

template <>
struct fnv_hash<uint32_t, FNV_1> : fnv_hash_base
{
    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint32_t hash(It begin, It end,
        uint32_t hashval = FNV1A_32_OFFSET_BASIS)
    {
        for(It current = begin; current != end; ++current)
        {
            hashval *= FNV1A_32_PRIME;
            hashval ^= static_cast<uint32_t>(*current);
        }

        return hashval;
    }
};


template <>
struct fnv_hash<uint32_t, FNV_1A> : fnv_hash_base
{
    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint32_t hash(It begin, It end,
        uint32_t hashval = FNV1A_32_OFFSET_BASIS)
    {
        for(It current = begin; current != end; ++current)
        {
            hashval ^= static_cast<uint32_t>(*current);
            hashval *= FNV1A_32_PRIME;
        }

        return hashval;
    }
};

template <>
struct fnv_hash<uint64_t, FNV_1> : fnv_hash_base
{
    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint64_t hash(It begin, It end, uint64_t hashval = FNV1_64_INIT)
    {
        It current = begin;
        for (; current != end; ++current)
        {
            hashval += shift_sum(hashval);
            hashval ^= static_cast<uint64_t>(*current);
        }
        return hashval;
    }
};

template <>
struct fnv_hash<uint64_t, FNV_1A> : fnv_hash_base
{
    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint64_t hash(It begin, It end, uint64_t hashval = FNV1_64_INIT)
    {
        It current = begin;
        for (; current != end; ++current)
        {
            hashval ^= static_cast<uint64_t>(*current);
            hashval += shift_sum(hashval);
        }
        return hashval;
    }
};

/*
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
 */

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
