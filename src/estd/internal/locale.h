// declarations
#pragma once

#include "charconv.h"

namespace estd { namespace experimental {

// https://docs.oracle.com/cd/E23824_01/html/E26033/glset.html

struct locale_code
{
    enum values
    {
        C,
        POSIX,
        en_GB,
        en_US,
        fr_FR
    };
};

typedef locale_code::values locale_code_enum;

template <locale_code_enum locale_code, internal::encodings::values encoding, class TChar>
class ctype;

struct locale_id
{
    locale_code::values code;
    internal::encodings::values encoding;
};

template <locale_code::values locale_code, internal::encodings::values encoding>
struct locale;

template <locale_code::values locale_code, internal::encodings::values encoding>
#if __cpp_constexpr
constexpr
#endif
const char* locale_name();

template <>
#if __cpp_constexpr
constexpr
#endif
const char* locale_name<locale_code::en_US, internal::encodings::UTF8>()
{
    return "en_US.UTF-8";
};

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
TFacet use_facet(const locale<locale_code, encoding>& loc);

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
void use_facet2(const locale<locale_code, encoding>& loc);

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
struct use_facet_helper;

// For internal use
typedef locale<locale_code::C, estd::internal::encodings::ASCII> classic_locale_type;

}}