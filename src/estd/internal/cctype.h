#pragma once

namespace estd { namespace internal {

template <class Char>
constexpr bool ascii_isupper(Char ch)
{
    return 'A' <= ch && ch <= 'Z';
}

template <class Char>
constexpr bool ascii_islower(Char ch)
{
    return 'a' <= ch && ch <= 'z';
}

// base MUST be <= 10 (hex support elsewhere in cbase)
template <class Char>
constexpr bool ascii_isdigit(Char ch, unsigned base = 10)
{
    return '0' <= ch && ch <= Char('0' + base - 1);
}

// Presumes incoming value is a letter (upper or lower)
template <class Char>
constexpr Char ascii_toupper(Char ch)
{
    return ch & ~0x20;
}

// Presumes incoming value is a letter (upper or lower)
template <class Char>
constexpr Char ascii_tolower(Char ch)
{
    return ch | 0x20;
}

// As per http://en.cppreference.com/w/cpp/string/byte/isspace
// *Probably* resilient to non-ASCII environments
template <class Char>
ESTD_CPP_CONSTEXPR(14) bool ascii_isspace(Char ch)
{
    switch(ch)
    {
        case ' ':
        case 13:
        case 10:
        case '\f':
        case '\t':
        case '\v':
            return true;

        default:
            return false;
    }
}


}}