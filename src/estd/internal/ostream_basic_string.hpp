#pragma once

#include "iosfwd.h"
#include "ostream_iterator.h"
#include "../string.h"

namespace estd {

namespace internal {

template <class Ostram, class Impl, class = void>
struct out_string_helper;

template <class Ostream, class Impl>
void out_string_helper_iterated(Ostream& out, const allocated_array<Impl>& str)
{
    typedef allocated_array<Impl> container_type;
    experimental::ostream_iterator<typename container_type::value_type, Ostream> o{out};

    estd::copy(str.begin(), str.end(), o);
}

template <class O, class Impl>
struct out_string_helper<O, Impl, typename estd::enable_if<
    Impl::allocator_traits::locking_preference == allocator_locking_preference::iterator>::type>
{
    static void out(O& out, const allocated_array<Impl>& str)
    {
        out_string_helper_iterated(out, str);
    }
};

template <class O, class Impl>
struct out_string_helper<O, Impl, typename estd::enable_if<
    !(Impl::allocator_traits::locking_preference == allocator_locking_preference::iterator)>::type>
{
    static void out(O& out, const allocated_array<Impl>& str)
    {
        out.write(str.clock(), str.size());
        str.cunlock();
    }
};


}

/*
template <class TStreambuf, class TBase, class TStringAllocator, class TStringPolicy>
inline detail::basic_ostream<TStreambuf, TBase>& operator <<(
        detail::basic_ostream<TStreambuf, TBase>& out,
        const estd::basic_string<
            typename TStreambuf::char_type,
            typename TStreambuf::traits_type,
            TStringAllocator,
            TStringPolicy >& str
        )
{
    out.write(str.clock(), str.size());

    str.cunlock();

    return out;
}   */

// catch-all case, intended for strings - though will work on any dynamic_array
template <class TStreambuf, class TBase, class TStringImpl>
inline detail::basic_ostream<TStreambuf, TBase>& operator <<(
    detail::basic_ostream<TStreambuf, TBase>& out,
    const internal::dynamic_array<TStringImpl>& str)
{
#if __cpp_static_assert
    // TODO: Would be better to pull out a 'streambuf_type'
    // and also check for type presence with a static_assert for both streambuf_type
    // and char_type
    typedef typename estd::remove_reference<TStreambuf>::type::char_type l_char_type;

    typedef typename internal::dynamic_array<TStringImpl>::value_type _r_char_type;
    typedef typename estd::remove_const<_r_char_type>::type r_char_type;

    static_assert (estd::is_same<l_char_type, r_char_type>::value, "character types must match");
#endif

    typedef internal::out_string_helper<detail::basic_ostream<TStreambuf, TBase>, TStringImpl> helper;

    helper::out(out, str);

    //out.write(str.clock(), str.size());
    //str.cunlock();
    return out;
}


// EXPERIMENTAL
/*
template <class TStreambuf, class TBase, class TImpl>
inline detail::basic_ostream<TStreambuf, TBase>& operator <<(
    detail::basic_ostream<TStreambuf, TBase>& out,
    const estd::experimental::private_array<TImpl>& pa
)
{
    typedef estd::experimental::private_array<TImpl> container_type;
    typedef typename container_type::iterator iterator;

    iterator it = pa.begin();
    iterator end = pa.end();

    while(it != end)    out.put(*it++);

    return out;
}   */

}
