#pragma once

// In support of:
// https://github.com/malachi-iot/estdlib/issues/166
// https://en.cppreference.com/w/cpp/iterator/size.html

#include "fwd/span.h"
#include "type_traits.h"
#include "fwd/utility.h"

#if FEATURE_STD_SPAN
#include <span>
#endif

namespace estd {

namespace internal {

template <class T, class Enabled = void>
struct is_container : false_type {};

//template <class Container, class Enabled = void>
//struct container_traits_base;

template <class T, class Enabled = void>
struct container_traits;


// Flag whether normal container detection ought to be bypassed.  This is useful for conditions where your
// container DOES conform to begin/end/size, but you want to control specializations anyway.  This
// is required because SFINAE can't easily differentiate between our container-detector and your own
// complete specialization.
// DEBT: I dislike excluding 'span' explicitly like this.  At least it is sequestered in a customizable
// bypass mechanism.
template <class T>
struct container_bypass : detail::is_span<T> {};

}

//template <class C>
//constexpr typename internal::container_traits<C>::iterator begin(C&);

namespace internal {

// 11JAN26 EXPERIMENTAL, kind of not liking container_traits for things like estd::data,
// estd::size though I do think there's utility in extent and value_type
// 30JAN26 Changing my mind.  Although a little bulky compared to pure freestanding functions,
// I like that parties can consume container_traits directly if they so choose

template <class T>
using is_present = is_void<void_t<T>>;

// Very crude is_container filter.  Not generally needed.  Catch2 does SFINAE range detection by
// looking up 'begin' which, at compile time, flips out container_traits since all kinds of non-container
// things are flung through it.  This mitigates that.  Feels like even in c++11 there is a more
// elegant way than this.
template <class T>
struct is_container<
    T,
    enable_if_t<
        is_present<typename T::iterator>::value &&
        is_present<decltype(std::declval<T&>().begin())>::value &&
        is_present<decltype(std::declval<T&>().end())>::value &&
        is_present<decltype(std::declval<T&>().size())>::value>> : true_type {};


// DEBT: Rename to container_traits_base once below one is renamed too
template <class Value, size_t Extent = detail::dynamic_extent::value>
struct container_traits_core
{
    ESTD_CPP_STD_VALUE_TYPE(Value)

    static constexpr size_t extent = Extent;
};

#if !__cpp_inline_variables
template <class Value, size_t Extent>
constexpr size_t container_traits_core<Value, Extent>::extent;
#endif

// Underlying feeder for begin, end, size
// DEBT: Rename to something like container_traits_standard
template <class Container>
struct container_traits_base :
    type_identity<Container>,
    container_traits_core<typename Container::value_type>
{
    using size_type = decltype(std::declval<Container>().size());
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    static constexpr iterator begin(Container& c)               { return c.begin(); }
    static constexpr iterator end(Container& c)                 { return c.end(); }
    static constexpr const_iterator begin(const Container& c)   { return c.begin(); }
    static constexpr const_iterator end(const Container& c)     { return c.end(); }

    static constexpr size_type size(const Container& c)
    {
        return c.size();
    }
};

template <class T>
struct container_traits<
    T,
    enable_if_t<container_bypass<T>::value == false && is_container<T>::value>> :
    container_traits_base<T> {};

template <class T, size_t N>
struct container_traits<estd::span<T, N>> :
    container_traits_base<estd::span<T, N>>
{
    static constexpr size_t extent = N;
};

#if FEATURE_STD_SPAN
template <class T, size_t N>
struct container_traits<std::span<T, N>, enable_if_t<(N > 0)>> :
    container_traits_base<std::span<T, N>>
{
    static constexpr size_t extent = N;
};
#endif

template <class T, size_t N>
struct container_traits<T[N]> : type_identity<T[N]>, container_traits_core<T, N>
{
    using size_type = size_t;
    using iterator = T*;
    using const_iterator = const T*;

    static constexpr iterator begin(T (&c)[N])              { return c; }
    static constexpr iterator end(T (&c)[N])                { return &c[N]; }
    static constexpr const_iterator begin(const T (&c)[N])  { return c; }
    static constexpr const_iterator end(const T (&c)[N])    { return &c[N]; }
    static constexpr size_type size(const T(&)[N])          { return N; }
};

}

#pragma push_macro("CTRAITS")
#define CTRAITS internal::container_traits<estd::remove_const_t<C>>

template <class C>
constexpr typename CTRAITS::iterator begin(C& c)
{
    return CTRAITS::begin(c);
}

template <class C>
constexpr typename CTRAITS::const_iterator begin(const C& c)
{
    return CTRAITS::begin(c);
}

template <class C>
constexpr typename CTRAITS::const_iterator cbegin(const C& c)
{
    return CTRAITS::begin(c);
}


template <class C>
constexpr typename CTRAITS::iterator end(C& c)
{
    return CTRAITS::end(c);
}


template <class C>
constexpr typename CTRAITS::const_iterator end(const C& c)
{
    return CTRAITS::end(c);
}


template <class C>
constexpr typename CTRAITS::const_iterator cend(const C& c)
{
    return CTRAITS::end(c);
}

template <class C>
constexpr auto data(const C& c) -> decltype(c.data())
{
    return c.data();
}

template <class T, size_t N>
ESTD_CPP_CONSTEXPR(17) T* data(T(&c)[N])
{
    return c;
}

template <class T, size_t N>
constexpr const T* data(const T(&c)[N])
{
    return c;
}


template <class C>
constexpr typename CTRAITS::size_type size(const C& c)
{
    return CTRAITS::size(c);
}

#pragma pop_macro("CTRAITS")

}
