#pragma once

#include "../iosfwd.h"
#include "../../traits/char_traits.h"

namespace estd {

namespace internal {

// Not used yet
struct ostream_assist
{
    template <class Streambuf, class Base>
    using os = detail::basic_ostream<Streambuf, Base>;

    template <class Streambuf, class Base>
    static os<Streambuf, Base>& leading(os<Streambuf, Base>& out)
    {
        if((out.flags() & ios_base::adjustfield) != ios_base::left) return out;

        // DEBT: fill_n silently eats < 0 so this is safe, but could be clearer.  Also this presumes
        // ios_base::left.  See https://github.com/malachi-iot/estdlib/issues/242
        out.fill_n(out.width() - 1);
        out.width(0);
        return out;
    }

    template <class Streambuf, class Base>
    static os<Streambuf, Base>& trailing(os<Streambuf, Base>& out)
    {
        return out;
    }
};

}

namespace detail {

template <class Streambuf, class Base>
enable_if_t<is_same<typename Base::char_type, char>::value == false, basic_ostream<Streambuf, Base>>& operator <<(
    basic_ostream<Streambuf, Base>& out,
    const char* s)
{
    using traits_type = char_traits<char>;
    using char_type = typename Base::char_type;

    // DEBT: Doesn't do width/fill alignment yet.  See https://github.com/malachi-iot/estdlib/issues/242

    ESTD_CPP_IF_CONSTEXPR(sizeof(char_type) == sizeof(char))
    {
        // 07OCT26 DEBT MB: Technically even matching character sizes ought to get the widen treatment.  However,
        // usually implicitly treating char as whatever 8-bit char_type is is probably good enough
        return out.write(reinterpret_cast<const char_type*>(s), traits_type::length(s));
    }
    else
    {
        // 07OCT26 DEBT MB: Explore ways to optimize this
        for(; *s; ++s)  out.put(out.widen(*s));
    }

    return out;
}

template <class Streambuf, class Base, typename T,
        class = enable_if_t<numeric_limits<T>::is_integer>>
ESTD_CPP_CONSTEXPR(14) basic_ostream<Streambuf, Base>&
    operator <<(basic_ostream<Streambuf, Base>& out, const T& value)
{
    // It's interesting that https://en.cppreference.com/cpp/io/basic_ostream/operator_ltlt doesn't mention
    // fill, width, padding, etc.
    return out_int_helper(out, value);
}


template <class Streambuf, class Base>
inline basic_ostream<Streambuf, Base>& operator <<(basic_ostream<Streambuf, Base>& out,
    typename Base::char_type ch)
{
    // DEBT: fill_n silently eats < 0 so this is safe, but could be clearer.  Also this presumes
    // ios_base::left.  See https://github.com/malachi-iot/estdlib/issues/242
    out.fill_n(out.width() - 1);
    out.width(0);

    return out.put(ch);
}

template <class Streambuf, class Base>
enable_if_t<is_same<typename Base::char_type, char>::value == false, basic_ostream<Streambuf, Base>>&
    operator <<(basic_ostream<Streambuf, Base>& out, char ch)
{
    //internal::ostream_assist::leading(out);

    out.fill_n(out.width() - 1);
    out.width(0);

    return out.put(out.widen(ch));
}

template <class Streambuf, class Base>
inline basic_ostream<Streambuf, Base>& operator <<(basic_ostream<Streambuf, Base>& out,
    bool v)
{
    // DEBT: Very crude.  Needs bounds check, width/padding, and maybe locale
    using char_type = typename Base::char_type;
    char_type s[16];
    num_put<char_type, char_type*> n;

    *n.put(s, out, ' ', v) = 0;

    return out << s;
}

}}  // namespace estd::detail