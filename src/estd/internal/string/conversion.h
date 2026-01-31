#pragma once

#include "../fwd/string.h"
#include "../charconv/features.h"
#include "../charconv/fwd.h"
#include "../charconv/result.h"

// DEBT: Clumsy suffix plurality, no corresponding estd/exceptions.h and dubious location
#ifndef FEATURE_ESTD_EXCEPTIONS
#define FEATURE_ESTD_EXCEPTIONS FEATURE_STD_EXCEPTION && __cpp_exceptions
#endif

#if FEATURE_ESTD_EXCEPTIONS
#include <exception>
#endif

namespace estd { namespace internal {

template <class Int, ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
Int stoi(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    Int v;
    using char_type = typename Impl::policy_type::char_traits::char_type;

    // DEBT: We'd like an optimization for (the frequent) null terminated string case to
    // avoid underlying strlen
    const char_type* data = str.data();
    const from_chars_result r = estd::from_chars<Int, true>(data, data + str.size(), v, base);

#if FEATURE_ESTD_EXCEPTIONS
    if(r.ec == errc::invalid_argument)
        throw std::invalid_argument("could not perform conversion");
    else if(r.ec == errc::result_out_of_range)
        throw std::out_of_range("overflow");

    if(pos)     *pos = r.ptr - data;
#else
    // Backchannel error status as '0' chars processed since we don't do exceptions
    if(pos)     *pos = r.ec == estd::errc{} ? (r.ptr - data) : 0;
#endif

    return v;
}

}

#if FEATURE_ESTD_FROM_CHARS_STOI
template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR(14) long stol(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    return internal::stoi<long>(str, pos, base);
}

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR(14) int stoi(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    return internal::stoi<int>(str, pos, base);
}

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR(14) unsigned long stoul(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    return internal::stoi<unsigned long>(str, pos, base);
}
#endif

}