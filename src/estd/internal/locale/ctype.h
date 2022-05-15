/**
 *
 * References:
 *
 * 1. cplusplus.com/reference/locale/ctype
 *
 */
#pragma once

#include "fwd.h"
#include "facet.h"

#include "../charconv.hpp"

#include "cbase.h"

namespace estd {

struct ctype_base
{
    typedef uint8_t mask;

    static CONSTEXPR mask space = 0x01;
    static CONSTEXPR mask digit = 0x02;
    static CONSTEXPR mask alpha = 0x04;
    static CONSTEXPR mask punct = 0x08;
    static CONSTEXPR mask upper = 0x10;
    static CONSTEXPR mask lower = 0x20;
    static CONSTEXPR mask xdigit = 0x40;

    static CONSTEXPR mask alnum = alpha | digit;
    static CONSTEXPR mask graph = alnum | punct;
};



// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <internal::locale_code_enum locale_code>
class ctype<char, internal::locale<locale_code, estd::internal::encodings::ASCII>> :
    public ctype_base,
    // NOTE: This inherit-from-facet behavior, though prescribed by std spec, is not
    // used for us at this time
    public internal::locale<locale_code, estd::internal::encodings::ASCII>::facet
{
    typedef internal::locale<locale_code, estd::internal::encodings::ASCII> locale_type;

public:
    static bool isspace(char ch)
    {
        // as per http://en.cppreference.com/w/cpp/string/byte/isspace
        switch(ch)
        {
            case ' ':
            case 13:
            case 10:
            case '\f':
            case '\t':
            case '\v':
                return true;
        }

        return false;
    }

    static bool isupper(char ch)
    {
        return 'A' <= ch && ch <= 'Z';
    }

    static bool islower(char ch)
    {
        return 'a' <= ch && ch <= 'a';
    }

    typedef char char_type;

    //static locale::id id;

    char widen(char c) const { return c; }

    // "returns whether c belongs to any of the categories specified in bitmask m" [1]
    static bool is(mask m, char ch)
    {
        if(m & space)
        {
            if(isspace(ch)) return true;
        }
        if(m & xdigit)
        {
            if(cbase<char, 16, locale_type>::is_in_base(ch))
                return true;
        }
        else if(m & digit)
        {
            if(cbase<char, 10, locale_type>::is_in_base(ch))
                return true;
        }
        if(m & upper)
        {
            if(isupper(ch)) return true;
        }
        else if(m & lower)
        {
            if(islower(ch)) return true;
        }
        return false;
    }
};

template <class TLocale>
class ctype<wchar_t, TLocale> : public ctype_base
{
public:
    static wchar_t widen(char ch) { return ch; }
};


template <internal::locale_code_enum locale_code>
class ctype<char, internal::locale<locale_code, estd::internal::encodings::UTF8>> :
    public ctype<char, internal::locale<locale_code, estd::internal::encodings::ASCII>> {};

namespace internal {

template <class TChar, class TLocale>
struct use_facet_helper<ctype<TChar, void>, TLocale>
{
    typedef ctype<TChar, TLocale> facet_type;
    
    inline static facet_type use_facet(TLocale)
    {
        return facet_type();
    }
};

}

}