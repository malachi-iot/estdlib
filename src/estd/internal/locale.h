// declarations
#pragma once

#include "platform.h"
#include "locale/fwd.h"

namespace estd { namespace internal {

struct locale_id
{
    locale_codes code;
    internal::encodings encoding;
};

// locale_names generally correspond to what we see in Debian

template <locale_codes locale_code, internal::encodings encoding>
constexpr const char* locale_name();

template <>
constexpr const char* locale_name<locale_codes::en_US, internal::encodings::UTF8>()
{
    return "en_US.UTF-8";
};


template <>
constexpr const char* locale_name<locale_codes::en_US, internal::encodings::ASCII>()
{
    return "en_US";
};

template <>
constexpr const char* locale_name<locale_codes::C, internal::encodings::ASCII>()
{
    return "C";
};



struct locale_base_base
{
    typedef int category;

    static constexpr category none = 0x0000;
    static constexpr category ctype = 0x0001;
    static constexpr category numeric = 0x0002;
};


template <internal::locale_codes locale_code_, internal::encodings encoding_>
struct locale : locale_base_base
{
    struct facet
    {

    };

    // TODO: deviates in that standard version uses a std::string
    // I want my own std::string (beginnings of which are in experimental::layer3::string)
    // but does memory allocation out of our own GC-pool
    const char* name() const { return internal::locale_name<locale_code_, encoding_>(); }

    // EXPERIMENTAL
    static constexpr internal::locale_codes locale_code = locale_code_;
    static constexpr internal::encodings encoding = encoding_;
};

typedef locale<locale_codes::C, encodings::ASCII> classic_locale_type;


}}
