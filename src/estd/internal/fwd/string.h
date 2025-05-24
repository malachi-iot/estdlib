#pragma once

#include "dynamic_array.h"
#include "../../traits/char_traits.h"
#include "../raw/type_traits.h"
#include "../feature/std.h"
#include "../feature/string.h"
#include "../../flags.h"

#if FEATURE_STD_OSTREAM
#include <iosfwd>
#endif

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

#if FEATURE_STD_OSTREAM
template <class Char, class Traits, class Impl>
inline std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os,
    const basic_string<Impl>& str);
#endif


}


namespace internal {

enum class string_options
{
    none                = 0x00,
    null_terminated     = 0x01,             ///< when not set, string is explicitly sized or compile-time size known
    constant            = 0x02,
    uninitialized       = 0x04,             ///< normal behavior initializes to 0-length.  Enable this for pure uninitialized (=default all the way) NOT READY YET
};

ESTD_FLAGS(string_options)

template <class CharTraits, string_options o, class Size = size_t, class Enabled = void>
struct string_policy;

// We're using std::char_traits (aliased) when available, which doesn't handle const char
// so that's why we pass in Char in addition to Traits.  Then, string_options::constant is deduced
template <class Char, class Traits, string_options options = string_options::none, typename Size = size_t>
using string_policy_helper =
    string_policy<Traits,
        options | (is_const<Char>::value ? string_options::constant : string_options::none),
        Size>;

// Favor using detail::basic_string, but there are edge cases where
// Allocator/Policy is more convenient
template <class Allocator, class Policy>
using basic_string = detail::basic_string<
    internal::impl::dynamic_array<Allocator, Policy> >;

}

#ifdef FEATURE_STD_MEMORY
template<
    class CharT,
    class Traits = estd::char_traits<typename estd::remove_const<CharT>::type >,
    class Allocator = std::allocator<CharT>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy =
        internal::string_policy_helper<CharT, Traits>>
using basic_string = internal::basic_string<Allocator, StringPolicy>;
#endif

namespace layer1 {

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy =
        internal::string_policy_helper<Char, Traits,
            null_terminated ? internal::string_options::null_terminated : internal::string_options::none>>
class basic_string;

template <size_t N, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;

}

namespace layer2 {

template<class Char, size_t N, bool null_terminated = true,
    class Traits = estd::char_traits<typename remove_const<Char>::type>,
    ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy =
        internal::string_policy_helper<Char, Traits,
            null_terminated ? internal::string_options::null_terminated : internal::string_options::none>>

class basic_string;

template <size_t N = 0, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;

}

}
