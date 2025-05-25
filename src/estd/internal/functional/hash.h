#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../../cstdint.h"
#include "../raw/type_traits.h"

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

    // NOTE: This double-casting probably isn't needed, playing it safe since original code did this

    // Crude way to enforce that only 8 bit values come in as 'v'
    template <class T>
    static constexpr uint8_t cast8(T v)
    {
        static_assert(sizeof(T) == 1, "Only 8 bit values are permitted");

        return static_cast<uint8_t>(v);
    }

    template <class T>
    static constexpr uint32_t cast32(T v)
    {
        return static_cast<uint32_t>(cast8(v));
    }

    template <class T>
    static constexpr uint64_t cast64(T v)
    {
        return static_cast<uint64_t>(cast8(v));
    }
};

template <>
struct fnv_hash<uint32_t, FNV_1> : fnv_hash_base
{
    static constexpr uint32_t INIT = FNV1A_32_OFFSET_BASIS;

    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint32_t hash(It begin, It end,
        uint32_t hashval = INIT)
    {
        for(It current = begin; current != end; ++current)
        {
            hashval *= FNV1A_32_PRIME;
            hashval ^= cast32(*current);
        }

        return hashval;
    }
};


template <>
struct fnv_hash<uint32_t, FNV_1A> : fnv_hash_base
{
    static constexpr uint32_t INIT = FNV1A_32_OFFSET_BASIS;

    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint32_t hash(It begin, It end,
        uint32_t hashval = INIT)
    {
        for(It current = begin; current != end; ++current)
        {
            hashval ^= cast32(*current);
            hashval *= FNV1A_32_PRIME;
        }

        return hashval;
    }
};

template <>
struct fnv_hash<uint64_t, FNV_1> : fnv_hash_base
{
    static constexpr uint64_t INIT = FNV1_64_INIT;

    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint64_t hash(It begin, It end, uint64_t hashval = INIT)
    {
        It current = begin;
        for (; current != end; ++current)
        {
            hashval += shift_sum(hashval);
            hashval ^= cast64(*current);
        }
        return hashval;
    }
};

template <>
struct fnv_hash<uint64_t, FNV_1A> : fnv_hash_base
{
    static constexpr uint64_t INIT = FNV1A_64_INIT;

    template <class It>
    ESTD_CPP_CONSTEXPR(14) static uint64_t hash(It begin, It end, uint64_t hashval = INIT)
    {
        It current = begin;
        for (; current != end; ++current)
        {
            hashval ^= cast64(*current);
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

// DEBT: A little too permissive, but probably OK.  Should only do arithmetic, enum and pointer as per
// https://en.cppreference.com/w/cpp/utility/hash
template <class T, class Enabled = enable_if_t<is_integral<T>::value>>
struct integral_hash
{
    constexpr size_t operator()(const T& v) const
    {
        return static_cast<size_t>(v);
    }
};


// 'Container' must be composed of 8-bit values
template <typename Precision = uint32_t>
struct container_hash
{
    // TODO: Put a 'concept' in here.  Not quite debt since this is 'internal' namespace *and*
    // the name of the struct is container_hash
    template <class Container>
    constexpr size_t operator()(const Container& c) const
    {
        return fnv_hash<Precision>::hash(c.begin(), c.end());
    }
};

}

// DEBT: A little too permissive, spec indicates we only should specialize the particular arithmetic
// ones, etc. where this attempts (but will fail) to be more broad
template <class T>
struct hash : internal::integral_hash<T> {};

}
