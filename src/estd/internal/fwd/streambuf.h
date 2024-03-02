#pragma once

// DEBT: Do a proper forward for char_traits -- at the moment we can't due to a possible 'using' scenario
#include "../../traits/char_traits.h"
#include "string.h"     // Only for CharTraits concept

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace estd {

// traditional basic_streambuf, complete with virtual functions
template<class Char, class Traits = estd::char_traits<Char> >
struct basic_streambuf;

#if __cpp_concepts
namespace concepts { inline namespace v1 {

// Streambuf impls have a more minimum requirement, since estd::detail::streambuf wraps it and adds more
namespace impl {

template <class T>
concept StreambufBase = requires(T sb)
{
    typename T::char_type;
    typename T::int_type;
    typename T::traits_type;
    requires CharTraits<typename T::traits_type>;
};

template <class T>
concept OutStreambuf = StreambufBase<T> && requires(T sb)
{
#if __cpp_lib_concepts
    // DEBT: Use / make an estd::convertible_to
    { sb.xsputn((typename T::char_type*){}, 0) } -> std::convertible_to<size_t>;
#endif
};


template <class T>
concept InStreambuf = StreambufBase<T> && requires(T sb)
{
    sb.xsgetn(nullptr, 0);
};

}

template <class T>
concept OutStreambuf = impl::StreambufBase<T> &&
    requires(T sb, T::char_type c)
{
    sb.sputn(&c, 1);
    sb.sputc(c);
};

template <class T>
concept InStreambuf = impl::StreambufBase<T> &&
    requires(T sb, T::char_type c)
{
    sb.sgetn(&c, 1);
};


}}
#endif

namespace internal {

// all the noop/defaults which we expect to hide/overload are now located in impl
// this struct now amounts to a type tag
struct streambuf_baseline { };


// TODO: TPolicy shall specify modes:
// 1. never blocking
// 2. blocking with timeout (including infinite timeout)
// 3. blocking
// Note that consuming istream/ostream may independently implement its own timeout code
// in which case 'never blocking' mode may be utilized for streambuf
// DEBT: Dedup from implementation comments - not doing so yet as we alias out a 'detail' flavor
template<class Impl, class Policy = void>
class streambuf;

}

namespace detail {

#if __cpp_alias_templates
inline namespace v1 {
template<ESTD_CPP_CONCEPT(concepts::v1::impl::StreambufBase) Impl, class Policy = void>
using streambuf = internal::streambuf<Impl, Policy>;
}
#endif

}

}