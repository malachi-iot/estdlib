#pragma once

// DEBT: Do a proper forward for char_traits -- at the moment we can't due to a possible 'using' scenario
#include "../../traits/char_traits.h"
#include "../feature/streambuf.h"
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

template <class T>
concept StreambufTraits = CharTraits<T> && requires
{
    typename T::signal;
};

// Streambuf impls have a more minimum requirement, since estd::detail::streambuf wraps it and adds more
namespace impl {

// NOTE: This particular constraint might want to sit outside 'impl'
template <class Raw, class T = estd::remove_reference_t<Raw> >
concept StreambufBase =
    // Not quite working
#if FEATURE_ESTD_STREAMBUF_TRAITS_
    StreambufTraits<typename T::traits_type>
#else
    CharTraits<typename T::traits_type>
#endif
    && requires(T sb)
{
    typename T::char_type;
    typename T::int_type;
    typename T::traits_type;
};

// These impl concepts are used somewhat per-function in internal::streambuf.  This
// approach is needed so as to have visibility into the protected methods it is checking on

template <class T>
concept OutStreambuf = StreambufBase<T> && requires(T sb)
{
#if __cpp_lib_concepts
    // DEBT: Use / make an estd::convertible_to
    { sb.xsputn((typename T::char_type*){}, 0) } -> std::convertible_to<size_t>;
#endif
};


template <class T>
concept InStreambuf = StreambufBase<T> && requires(T sb,
    remove_const_t<typename T::char_type>* rhs)
{
    sb.xsgetn(rhs, 0);
    sb.xsgetc();
    sb.underflow();
};

}

template <class Raw, class T = remove_reference_t<Raw> >
concept OutStreambuf = impl::StreambufBase<T> &&
    requires(T sb, T::char_type c)
{
    sb.sputn(&c, 1);
    sb.sputc(c);
};

template <class Raw, class T = remove_reference_t<Raw>>
concept InStreambuf = impl::StreambufBase<T> &&
    requires(T sb, T::char_type c)
{
    sb.sgetn(&c, 1);
    sb.in_avail();
    sb.sgetc();
    sb.sbumpc();
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
// Only enforces fundamental impl streambuf concept since this MIGHT be input, output or both
template<ESTD_CPP_CONCEPT(concepts::v1::impl::StreambufBase) Impl, class Policy = void>
using streambuf = internal::streambuf<Impl, Policy>;

/*
// Enforces input concepts more aggressively.  Does not preclude output
template<ESTD_CPP_CONCEPT(concepts::v1::impl::InStreambuf) Impl, class Policy = void>
using istreambuf = internal::streambuf<Impl, Policy>;

// Enforces output concepts more aggressively.  Does not preclude input
template<ESTD_CPP_CONCEPT(concepts::v1::impl::OutStreambuf) Impl, class Policy = void>
using ostreambuf = internal::streambuf<Impl, Policy>;
 */
}
#endif

}

}