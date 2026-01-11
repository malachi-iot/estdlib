#pragma once

// In support of:
// https://github.com/malachi-iot/estdlib/issues/166
// https://en.cppreference.com/w/cpp/iterator/size.html

#include "iterator_standalone.h"

namespace estd {

namespace internal {

// 11JAN26 EXPERIMENTAL, kind of not liking container_traits for things like estd::data,
// estd::size though I do think there's utility in is_const_size

// Underlying feeder for begin, end, size
template <class Container>
struct container_traits_base : type_identity<Container>
{
    using size_type = decltype(std::declval<Container>().size());
    using iterator = typename Container::pointer;

    static ESTD_CPP_CONSTEXPR(17) iterator begin(Container& c) { return c.begin(); }
    static ESTD_CPP_CONSTEXPR(17) iterator end(Container& c) { return c.end(); }

    static constexpr size_type size(Container& c)
    {
        return c.size();
    }

    static constexpr size_t extent = detail::dynamic_extent::value;
};

template <class Container, class Enabled = void>
struct container_traits : container_traits_base<Container> {};

template <class T, size_t N>
struct container_traits<span<T, N>, enable_if_t<(N > 0)>> :
    container_traits_base<span<T, N>>
{
    static constexpr size_t extent = N;
};

template <class T, size_t N>
struct container_traits<T[N]> : type_identity<T[N]>
{
    using size_type = size_t;
    using iterator = T*;

    static ESTD_CPP_CONSTEXPR(17) iterator begin(T (&c)[N])
    {
        return c;
    }

    static ESTD_CPP_CONSTEXPR(17) iterator end(T (&c)[N])
    {
        return c;
    }

    static constexpr size_type size(const T(&)[N])
    {
        return N;
    }

    static constexpr size_t extent = N;
};

}

template <class C>
constexpr auto data(const C& c) -> decltype(c.data())
{
    return c.data();
}

template <class T, size_t N>
constexpr const T* data(const T(&c)[N])
{
    return c;
}


template <class C>
constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}

template <class T, size_t N>
ESTD_CPP_CONSTEVAL size_t size(const T(&)[N])
{
    return N;
}

}
