#pragma once

#include "dynamic_array.h"
#include "../../policy/string.h"
#include "../../traits/char_traits.h"
#include "../raw/type_traits.h"

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace estd {

#if __cpp_concepts
namespace concepts { inline namespace v1 {

template <class T>
concept CharTraits = requires
{
    typename T::char_type;
    typename T::int_type;

    T::eof();
    T::to_int_type(typename T::char_type{});
};

namespace impl {

template <class T> //, class Traits = estd::char_traits<char> >
concept String = impl::Container<T> && internal::AllocatedArrayImpl<T> &&
requires(T s, const typename T::value_type* rhs)
{
    typename T::value_type;

    // DEBT: Get these online
    //typename T::traits_type;
    //{ s.compare(rhs) };
};

}

template <class T>
concept String = //impl::String<T>
    Container<T> &&
    requires(T s, typename T::pointer rhs)
{
    typename T::traits_type;

    requires CharTraits<typename T::traits_type>;

    s.starts_with(rhs);
    //s.operator =(rhs);
};

}}
#endif

namespace internal {

#if __cpp_concepts
template <class T>
concept StringPolicy = BufferPolicy<T> &&
requires
{
    typename T::char_traits;

    // Only available when NOT in the strict mode
    //T::is_null_terminated();
};
#endif

template <class Char, unsigned N, class CharTraits = estd::char_traits<Char> >
class shifted_string;

}

namespace detail {

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
class basic_string;

}


/*
template<
    class CharT,
    class Traits,
    class Allocator,
    ESTD_CPP_CONCEPT(internal::StringPolicy) Policy
> class basic_string;
*/

namespace internal {

// DEBT: Clumsy
// We're using std::char_traits (aliased) when available, which doesn't handle const char
// so that's why we pass in Char in addition to Traits
template <class Char, class Traits, bool null_terminated>
using string_policy_helper = conditional_t<null_terminated,
        null_terminated_string_policy<Traits, int16_t, is_const<Char>::value>,
        sized_string_policy<Traits, int16_t, is_const<Char>::value>>;

// Phase this out in favor of always using detail::basic_string
// Keeping for the edge cases where Allocator/Policy is more convenient
template <class Allocator, class Policy>
using basic_string = detail::basic_string<
    internal::impl::dynamic_array<Allocator, Policy> >;

}

template<
    class CharT,
    class Traits = estd::char_traits<typename estd::remove_const<CharT>::type >,
#ifdef FEATURE_STD_MEMORY
    class Allocator = std::allocator<CharT>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy = internal::sized_string_policy<Traits>
#else
    class Allocator, ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy
#endif
>
using basic_string = internal::basic_string<Allocator, StringPolicy>;

namespace layer1 {

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy = internal::string_policy_helper<Char, Traits, null_terminated>>
class basic_string;

template <size_t N, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;

}

namespace layer2 {

template<class Char, size_t N, bool null_terminated = true,
    class Traits = estd::char_traits<typename remove_const<Char>::type>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy = internal::string_policy_helper<Char, Traits, null_terminated>>
class basic_string;

}

}
