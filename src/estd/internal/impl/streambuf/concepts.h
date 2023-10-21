#pragma once

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

template <class T>
concept Streambuf = impl::Streambuf<T> && requires(T s)
{
    //s.sputn((typename T::char_type*){}, streamsize{});
    //s.sgetn(nullptr, streamsize{});
    //{ s.sgetc() } -> std::convertible_to<typename T::int_type>;
    s.sgetc();
};

}}

#endif