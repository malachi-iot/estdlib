#pragma once

#include "../type_traits.h"

namespace estd {

namespace internal {

// since < c++11 can't do static constexpr bool functions, we need an alternate
// way to build up specializations based on attribute_traits.  Also has benefit
// of easier SFINAE application for allocators who don't have tags present

// locking_tag tells us two things:
// 1) that locking calls are required to acquire references to underlying data
// 2) that the requisite locking API is present
//
// if locking_tag is not present, the API may still optionally be there, but is
// not required as handle_type will more or less be pointer type
// our estd::allocator_traits will always provide a lock/unlock API, even if it's
// basically a noop
//
// FIX: note also that locking actually carries two distinct functionalities:
// 1) conversion from a handle to a memory location
// 2) isolation and access to memory location
//
// these two operations have occasionally divergent use cases, i.e. it's reasonable to grab a lingering
// reference to a fixed memory location through a handle, but not a lingering reference to a possibly
// moving/gc'd memory location.  so with that, we need to revise tags to reflect those 2 distinct cases
template<typename T, typename = void>
struct has_locking_tag : estd::false_type {};

template<typename T>
struct has_locking_tag<T, typename estd::internal::has_typedef<typename T::is_locking_tag>::type> : estd::true_type {};

ESTD_FN_HAS_TYPEDEF_EXP(is_pinned_tag_exp)
// NOTE: Not yet active, following 'pinned' - replaces above has typedef
// (has tag is a more tailored version, hopefully easier to read and use)
// currently differs from prior paradigm in that underlying typedef is
// expected to be 'pinned_tag' and not 'is_pinned_tag'.  Undecided if that's
// my preference, so yet another reason this is experimental
ESTD_FN_HAS_TAG_EXP(pinned)

template<typename T, typename = void>
struct has_stateful_tag : estd::false_type {};

template<typename T>
struct has_stateful_tag<T, typename estd::internal::has_typedef<typename T::is_stateful_tag>::type> : estd::true_type {};

template<typename T, typename = void>
struct has_singular_tag : estd::false_type {};

template<typename T>
struct has_singular_tag<T, typename estd::internal::has_typedef<typename T::is_singular_tag>::type> : estd::true_type {};

template<typename T, typename = void>
struct has_size_tag : estd::false_type {};

template<typename T>
struct has_size_tag<T, typename estd::internal::has_typedef<typename T::has_size_tag>::type> : estd::true_type {};

template<typename T, typename = void>
struct has_noncontiguous_tag : estd::false_type {};

template<typename T>
struct has_noncontiguous_tag<T, typename estd::internal::has_typedef<typename T::is_noncontiguous_tag>::type> : estd::true_type {};


template<typename T, typename = void>
struct has_difference_type : estd::false_type {};

template<typename T>
struct has_difference_type<T, typename estd::internal::has_typedef<typename T::difference_type>::type> : estd::true_type {};


template<typename TAlloc, typename = void>
struct get_difference_type
{
    typedef typename estd::pointer_traits<typename TAlloc::pointer>::difference_type type;
};

template<typename TAlloc>
struct get_difference_type<TAlloc,
    typename estd::internal::has_typedef<typename TAlloc::difference_type>::type>
{
    typedef typename TAlloc::difference_type type;
};

}

}