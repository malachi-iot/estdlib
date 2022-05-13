#pragma once

#include "fwd.h"
#include "facet.h"

namespace estd { namespace experimental {

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


template <typename TChar, class TLocale>
class ctype : public ctype_base
{
#ifdef ENABLE_LOCALE_MULTI
    // TODO: determine if we want to roll with the virtual function do_is
    // and friends or branch out into further templating
#else
    TChar do_tolower(TChar ch);
    TChar do_toupper(TChar ch);
#endif
public:
    bool is(mask m, TChar ch) const { return false; }
    const TChar* is(const TChar* low, const TChar* high, mask* vec) const { return NULLPTR; }

    TChar toupper(TChar ch) { return do_toupper(ch); }
    TChar tolower(TChar ch) { return do_tolower(ch); }
};


// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <locale_code_enum locale_code>
class ctype<char, locale<locale_code, estd::internal::encodings::ASCII>> :
    public ctype_base,
    public locale<locale_code, estd::internal::encodings::ASCII>::facet
{
public:
    typedef char char_type;

    //static locale::id id;

    char widen(char c) const { return c; }

    bool is(mask m, char ch) const
    {
        if(m & space)
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
        }
        if(m & digit)
        {
            if(ch >= '0' && ch <= '9') return true;
        }
        return false;
    }
};

template <locale_code_enum locale_code>
class ctype<char, locale<locale_code, estd::internal::encodings::UTF8>> :
    public ctype<char, locale<locale_code, estd::internal::encodings::ASCII>> {};

template <class TChar, class TLocale>
struct use_facet_helper4<ctype<TChar, void>, TLocale>
{
    typedef ctype<TChar, TLocale> facet_type;
    
    inline static facet_type use_facet(TLocale)
    {
        return facet_type();
    }
};


}}