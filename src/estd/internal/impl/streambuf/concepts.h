#pragma once

#include "../../raw/iosfwd.h"

#if __cpp_concepts

namespace estd { namespace internal {

namespace impl {

template <class T>
concept Streambuf = requires(T s)
{
    typename T::traits_type;
    typename T::char_type;

    //{ s.showmanyc() } -> std::convertible_to<streamsize>;
    //s.showmanyc();
};

}

// TODO: Break out into InputStreambuf, and somehow combine those together
// for consuming scenarios (maybe retain this pseudo-combined flavor since
// our streambuf feeder which everyone is gonna use implements 99% of the API,
// even if impl is lacking)
template <class T>
concept Streambuf = impl::Streambuf<T> && requires(T s)
{
    typename T::int_type;
    typename T::pos_type;

    s.sputn((typename T::char_type*){}, estd::streamsize{});
    s.sgetn(nullptr, estd::streamsize{});
    { s.sgetc() } -> std::convertible_to<typename T::int_type>;
    s.in_avail();

    // sputc is very implementation dependent and a lot of ostreambufs just don't have it
};

template <class T>
concept OutputStreambuf = Streambuf<T> && requires(T s)
{
    { s.sputc(typename T::char_type{}) };
};

}}

#endif