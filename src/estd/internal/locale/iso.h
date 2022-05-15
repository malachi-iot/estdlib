#pragma once

namespace estd { namespace internal {

// https://docs.oracle.com/cd/E23824_01/html/E26033/glset.html

struct locale_code
{
    enum values
    {
        C,
        POSIX,
        en_GB,
        en_US,
        fr_FR,
        ja_JP
    };
};

typedef locale_code::values locale_code_enum;

}}