// Non-standard facet for character-to-numeric base conversion
#pragma once

//#include <estd/string.h>
#include <estd/optional.h>

#include "fwd.h"
#include "facet.h"
#include "utility.h"
#include "../deduce_fixed_size.h"
#include "../cctype.h"

// TODO: A future task.  Japanese character set has 3 different (at least) sets of numerals:
// 1.  Regular ASCII style
// 2.  Kanji style
// 3.  An Arabic (English) version which appears to be in with the other Kanji characters

namespace estd {

//template <typename TChar, unsigned b, class TLocale>
//struct cbase;

namespace internal {

template <typename TChar, unsigned b, typename = estd::internal::Range<true> >
struct cbase_utf;

template <typename TChar, unsigned b>
struct cbase_utf_base
{
    typedef int16_t int_type;

    //typedef int_type optional_type;
    //inline static CONSTEXPR int_type eol() { return -1; }

    typedef const estd::layer1::optional<int_type, -1> optional_type;

    inline static CONSTEXPR int_type eol() { return optional_type::null_value(); }

    static inline CONSTEXPR unsigned base() { return b; }
};

template <typename TChar, unsigned b>
struct cbase_utf<TChar, b, estd::internal::Range<b <= 10> > :
    cbase_utf_base<TChar, b>
{
    typedef cbase_utf_base<TChar, b> base_type;
    typedef typename base_type::int_type int_type;
    typedef TChar char_type;

    // adapted from GNUC
    static ESTD_CPP_CONSTEXPR_RET bool is_in_base(char_type c, const unsigned _base = b)
    {
        return '0' <= c && c <= ('0' + (char_type)(_base - 1));
    }

    /// No bounds checking performed on conversion.
    /// \param c
    /// \return Character value of 0-9 converted from char to int.  Any other character value
    /// results in an int_type either < 0 or > 10
    static ESTD_CPP_CONSTEXPR_RET int_type from_char_raw(char_type c)
    {
        return c - '0';
    }

    static ESTD_CPP_CONSTEXPR_RET char_type to_char(int_type v)
    {
        return '0' + v;
    }

    static ESTD_CPP_CONSTEXPR_RET typename base_type::optional_type
    from_char(char_type c, const int _base = b)
    {
        return is_in_base(c, _base) ?
               from_char_raw(c) :
               //typename base_type::optional_type::null_value();
               base_type::eol();
    }
};


// DEBT: In fact, really we have only two cbase_utfs - a base 10 one and a base 36 one.
// On one level, this range is almost overly fancy and showing off when we could merely
// have specialized on b directly.
// On the other hand, there is a convenience here in that consumers knowing at compile time
// what base they are using can auto-feed 'base' to things like 'from_chars_integer'.
template <typename TChar, unsigned b>
struct cbase_utf<TChar, b, estd::internal::Range<(b > 10 && b <= 36)> > :
    cbase_utf_base<TChar, b>
{
    typedef cbase_utf_base<TChar, b> base_type;
    typedef typename base_type::int_type int_type;
    typedef TChar char_type;

    static ESTD_CPP_CONSTEXPR_RET bool isupper(char_type c, const unsigned _base = b)
    {
        return 'A' <= c && c <= ('A' + (char_type)(_base - 11));
    }

    static ESTD_CPP_CONSTEXPR_RET bool islower(char_type c, const unsigned _base = b)
    {
        return 'a' <= c && c <= ('a' + (char_type)(_base - 11));
    }

    static inline CONSTEXPR bool is_in_base(char_type c, const unsigned _base = b)
    {
        // DEBT: We really want to consider ctype's isdigit here
        return estd::internal::ascii_isdigit(c) ||
               isupper(c, _base) ||
               islower(c, _base);
    }

    static inline typename base_type::optional_type
    from_char(char_type c, const unsigned short _base = b)
    {
        // DEBT: We really want to consider ctype's isdigit, toupper and islower here
        if (estd::internal::ascii_isdigit(c)) return c - '0';

        if (isupper(c, _base)) return c - 'A' + 10;

        if (islower(c, _base)) return c - 'a' + 10;

        return estd::nullopt;
    }

    static inline int_type from_char_raw(char_type c)
    {
        if (c <= '9')
            return c - '0';
        else if (c <= 'Z')
            return c - 'A' + 10;
        else
            return c - 'a' + 10;
    }

    static inline CONSTEXPR char_type to_char(int_type v)
    {
        return v <= 10 ?
               ('0' + v) :
               'a' + (v - 10);
    }
};


template <typename TChar, unsigned b,
    internal::locale_code::values lc,
    internal::encodings::values encoding>
struct cbase<TChar, b, internal::locale<lc, encoding>,
    typename estd::enable_if<
        encoding == internal::encodings::ASCII ||
        encoding == internal::encodings::UTF8 ||
        encoding == internal::encodings::UTF16>::type> :
    cbase_utf<TChar, b>
{

};

template <class TChar, unsigned b, class TLocale>
struct use_facet_helper<estd::cbase<TChar, b, void>, TLocale>
{
    typedef estd::cbase<TChar, b, TLocale> facet_type;

    inline static facet_type use_facet(TLocale)
    {
        return facet_type();
    }
};

}

}