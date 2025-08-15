/**
 *
 * References:
 *
 * 1. cplusplus.com/reference/locale/ctype
 * 2. https://en.cppreference.com/w/cpp/locale/ctype_char/is.html
 *
 */
#pragma once

#include "cbase.h"
#include "facet.h"

namespace estd {

struct ctype_base
{
    using mask = uint8_t;

    static constexpr mask space = 0x01;
    static constexpr mask digit = 0x02;
    static constexpr mask alpha = 0x04;
    static constexpr mask punct = 0x08;
    static constexpr mask upper = 0x10;
    static constexpr mask lower = 0x20;
    static constexpr mask xdigit = 0x40;

    // Not yet used - see https://en.cppreference.com/w/cpp/string/byte/isblank.html
    static constexpr mask blank = 0x80;

    static constexpr mask alnum = alpha | digit;
    static constexpr mask graph = alnum | punct;
};

namespace internal {

template <class Char>
class ascii_ctype_base : public ctype_base
{
public:
    using char_type = Char;

    // These discrete helpers are not part of std
    static constexpr bool isspace(char_type ch) { return internal::ascii_isspace(ch); }
    static constexpr bool isupper(char_type ch) { return internal::ascii_isupper(ch); }
    static constexpr bool islower(char_type ch) { return internal::ascii_islower(ch); }
    static constexpr bool isdigit(char_type ch) { return internal::ascii_isdigit(ch); }

    static constexpr char_type toupper(char_type ch)
    {
        return internal::ascii_islower(ch) ?
            internal::ascii_toupper(ch) :
            ch;
    }

    static constexpr char_type tolower(char_type ch)
    {
        return internal::ascii_isupper(ch) ?
            internal::ascii_tolower(ch) :
            ch;
    }
};

// DEBT: shoulders char and wchar_t burden - which is fine - but ought to filter
// by those
// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <class Char, class Locale>
class ctype<Char, Locale,
    enable_if_t<internal::is_compatible_with_classic_locale<Locale>::value>> :
    public ascii_ctype_base<Char>
{
    using base_type = ascii_ctype_base<Char>;
    using locale_type = Locale;

public:
    using typename base_type::mask;
    using typename base_type::char_type;

    //static locale::id id;

    static constexpr char_type widen(char c) { return c; }

    // "returns whether c belongs to any of the categories specified in bitmask m" [1]
    // 06JUL25 MB DEBT: Serves me right for using cplusplus.com, probably it's a bit more mutually
    // exclusive (not sure) as per [2]
    static ESTD_CPP_CONSTEXPR(14) bool is(mask m, char_type ch)
    {
        if(m & base_type::space)
        {
            if(base_type::isspace(ch)) return true;
        }
        if(m & base_type::xdigit)
        {
            if(estd::cbase<char_type, 16, locale_type>::is_in_base(ch))
                return true;
        }
        else if(m & base_type::digit)
        {
            if(base_type::isdigit(ch))
                return true;
        }
        if(m & base_type::upper)
        {
            if(base_type::isupper(ch)) return true;
        }
        else if(m & base_type::lower)
        {
            if(base_type::islower(ch)) return true;
        }
        return false;
    }
};

}


template <class Char, class Locale>
class ctype : public internal::ctype<Char, Locale>
{
    using base_type = internal::ctype<Char, Locale>;

public:
    using base_type::tolower;
    using base_type::toupper;

    static ESTD_CPP_CONSTEXPR(14) Char* toupper(Char* beg, Char* end)
    {
        for(;beg != end; ++beg)   *beg = toupper(*beg);
        return beg;
    }

    static ESTD_CPP_CONSTEXPR(14) Char* tolower(Char* beg, Char* end)
    {
        for(;beg != end; ++beg)   *beg = toupper(*beg);
        return beg;
    }
};

namespace internal {

template <class Char, class Locale>
struct use_facet_helper<estd::ctype<Char, void>, Locale>
{
    using facet_type = estd::ctype<Char, Locale>;
    
    constexpr static facet_type use_facet(Locale)
    {
        return facet_type{};
    }
};

}

}
