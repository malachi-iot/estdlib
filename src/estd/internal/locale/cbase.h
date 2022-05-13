// Non-standard facet for character-to-numeric base conversion
#pragma once

#include "fwd.h"
#include "facet.h"
#include "utility.h"
#include "../deduce_fixed_size.h"
#include <estd/string.h>

// TODO: A future task.  Japanese character set has 3 different (at least) sets of numerals:
// 1.  Regular ASCII style
// 2.  Kanji style
// 3.  An Arabit (English) version which appears to be in with the other Kanji characters

namespace estd { namespace experimental {

//template <typename TChar, unsigned b, class TLocale>
//struct cbase;

namespace _internal {

template <typename TChar, unsigned b, class TLocale, typename TEnabled = void>
struct cbase;

template <typename TChar, unsigned b, typename = estd::internal::Range<true> >
struct cbase_utf;


template <typename TChar, unsigned b>
struct cbase_utf<TChar, b, estd::internal::Range<b <= 10> >
{
    typedef int16_t int_type;
    typedef TChar char_type;
    
    static inline CONSTEXPR int_type from_char(char_type c)
    {
        return c - '0';
    }
};


template <typename TChar, unsigned b>
struct cbase_utf<TChar, b, estd::internal::Range<(b > 10 && b <= 36)> >
{
    typedef int16_t int_type;
    typedef TChar char_type;

    static inline int_type from_char(char_type c)
    {
        if (c <= '9')
            return c - '0';
        else if (c <= 'Z')
            return c - 'A' + 10;
        else
            return c - 'a' + 10;
    }
};


template <typename TChar, unsigned b,
    locale_code::values lc,
    internal::encodings::values encoding>
struct cbase<TChar, b, locale<lc, encoding>,
    typename estd::enable_if<
        encoding == internal::encodings::ASCII ||
        encoding == internal::encodings::UTF8 ||
        encoding == internal::encodings::UTF16>::type> :
    cbase_utf<TChar, b>
{

};

}

template <typename TChar, unsigned b, class TLocale>
using cbase = _internal::cbase<TChar, b, TLocale>;


template <class TChar, unsigned b, class TLocale>
struct use_facet_helper4<cbase<TChar, b, void>, TLocale>
{
    typedef cbase<TChar, b, TLocale> facet_type;

    inline static facet_type use_facet(TLocale)
    {
        return facet_type();
    }
};


}}