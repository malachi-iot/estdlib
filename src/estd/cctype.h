#pragma once

// DEBT: All these expect ASCII "C" locale
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
    return ch == ' ';
}

inline int isdigit(int ch)
{
    return '0' <= ch && ch <= '9';
}

}
