// declarations
#pragma once

#include "charconv.h"
#include "locale/fwd.h"

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

struct locale_id
{
    locale_code::values code;
    internal::encodings::values encoding;
};

#if __cpp_constexpr
#define ESTD_CPP_CONSTEXPR_RET constexpr
#else
#define ESTD_CPP_CONSTEXPR_RET inline
#endif

// locale_names generally correspond to what we see in Debian

template <locale_code::values locale_code, internal::encodings::values encoding>
ESTD_CPP_CONSTEXPR_RET const char* locale_name();

template <>
ESTD_CPP_CONSTEXPR_RET const char* locale_name<locale_code::en_US, internal::encodings::UTF8>()
{
    return "en_US.UTF-8";
};


template <>
ESTD_CPP_CONSTEXPR_RET const char* locale_name<locale_code::en_US, internal::encodings::ASCII>()
{
    return "en_US";
};

template <>
ESTD_CPP_CONSTEXPR_RET const char* locale_name<locale_code::C, internal::encodings::ASCII>()
{
    return "C";
};

#if UNUSED
template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
TFacet use_facet(const locale<locale_code, encoding>& loc);

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
void use_facet2(const locale<locale_code, encoding>& loc);

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
struct use_facet_helper;
#endif


struct locale_base_base
{
    typedef int category;

    static CONSTEXPR category none = 0x0000;
    static CONSTEXPR category ctype = 0x0001;
    static CONSTEXPR category numeric = 0x0002;
};


template <internal::locale_code::values locale_code, internal::encodings::values encoding>
struct locale;


typedef locale<locale_code::C, estd::internal::encodings::ASCII> classic_locale_type;


}

}