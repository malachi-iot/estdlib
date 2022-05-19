#pragma once

#include "internal/cctype.h"
#include "internal/locale/ctype.h"

// DEBT: Revamp all these to use locale's ctype
namespace estd {

inline int isupper(int ch)
{
    return 'A' <= ch && ch <= 'Z';
}

inline int islower(int ch)
{
    return 'a' <= ch && ch <= 'z';
}

inline int isalpha(int ch)
{
    return isupper(ch) || islower(ch);
}

inline int isspace(int ch)
{
    return internal::ascii_isspace(ch);
}

inline int isdigit(int ch)
{
    return internal::ascii_isdigit(ch);
}

}
