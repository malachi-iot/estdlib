// declarations
#pragma once

#include "locale/fwd.h"

namespace estd { namespace internal {

struct locale_id
{
    locale_code::values code;
    internal::encodings::values encoding;
};

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



struct locale_base_base
{
    typedef int category;

    static CONSTEXPR category none = 0x0000;
    static CONSTEXPR category ctype = 0x0001;
    static CONSTEXPR category numeric = 0x0002;
};


template <internal::locale_code::values locale_code, internal::encodings::values encoding>
struct locale : locale_base_base
{
    struct facet
    {

    };

    // TODO: deviates in that standard version uses a std::string
    // I want my own std::string (beginnings of which are in experimental::layer3::string)
    // but does memory allocation out of our own GC-pool
    const char* name() const { return internal::locale_name<locale_code, encoding>(); }
};

typedef locale<locale_code::C, encodings::ASCII> classic_locale_type;


}}