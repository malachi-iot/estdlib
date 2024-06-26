#pragma once

#include "dynamic_array.h"
#include "../../policy/string.h"
#include "../../traits/char_traits.h"

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

template <class Allocator, class Policy>
class basic_string;

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


template<
    class CharT,
    class Traits,
    class Allocator,
    ESTD_CPP_CONCEPT(internal::StringPolicy) Policy
> class basic_string;


namespace layer1 {

template<class CharT, size_t N, bool null_terminated = true, class Traits = estd::char_traits<CharT >,
         // DEBT: Move resolution for particular policy elsewhere
        ESTD_CPP_CONCEPT(internal::StringPolicy) StringPolicy = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_policy<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_policy<Traits, int16_t, estd::is_const<CharT>::value> >::type
                >
class basic_string;


#if __cpp_alias_templates
template <size_t N, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;
#else
template <size_t N, bool null_terminated = true>
class string : public basic_string<char, N, null_terminated>
{
    typedef basic_string<char, N, null_terminated> base_t;

public:
    string() {}

    string(const char* s) : base_t(s)
    {
    }
};
#endif

}

}
