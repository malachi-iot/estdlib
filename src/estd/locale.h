#pragma once

#include "internal/platform.h"
#include "internal/locale.h"
#include "internal/iterator_standalone.h"
#include "internal/ios.h"
#include "internal/charconv.hpp"

#include "internal/iosfwd.h"

#include "string.h"

#include "internal/locale/ctype.h"
#include "internal/locale/money.h"
#include "internal/locale/num_get.h"
#include "internal/locale/numpunct.h"

extern "C" {
#include <stdint.h>
}

namespace estd { namespace experimental {

struct locale_base_base
{
    typedef int category;

    static CONSTEXPR category none = 0x0000;
    static CONSTEXPR category ctype = 0x0001;
    static CONSTEXPR category numeric = 0x0002;
};

template <locale_code::values locale_code, internal::encodings::values encoding>
struct locale : locale_base_base
{
#ifdef ENABLE_LOCALE_MULTI
    struct facet
    {

    };

    typedef int id;

    // FIX: 40 arbitrary number, could be more or less
    // NOTE: seems kind of like a fake-rtti system a bit
    facet* facets[40];

    locale(const locale& other);
    explicit locale(const char* std_name);
#else
    struct facet
    {

    };

    struct id
    {

    };
#endif

    // TODO: deviates in that standard version uses a std::string
    // I want my own std::string (beginnings of which are in experimental::layer3::string)
    // but does memory allocation out of our own GC-pool
    const char* name() const { return locale_name<locale_code, encoding>(); }
};


// DEBT: Need to move this out to regular estd and rename to 'locale'
// so that calls like 'classic' have a std-like signature
// NOTE: Due to how we specialize, this class cannot be base class of our specialized locale
struct locale_base : locale_base_base
{
    inline static classic_locale_type classic() { return classic_locale_type(); }
};





template <class TChar>
class ctype_test;


namespace layer5
{

template <class TChar>
class ctype : public ctype_base
{
public:
    virtual bool do_is(mask m, TChar c) const;
};


}

//template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
//bool has_facet(const locale<locale_code, encoding>& loc);

/*
template<locale_code_enum locale_code, internal::encodings::values encoding>
inline bool has_facet<ctype<locale_code, encoding, char> >(const locale<locale_code, encoding>&)
{
    return true;
}

template<>
inline const ctype<char>& use_facet(const locale&)
{
    static ctype<char> facet;

    return facet;
} */

// FIX: Just to get things compiling, hardcoding these
template <class TFacet, class TLocale>
inline bool has_facet(const TLocale&) { return true; }



template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
inline TFacet use_facet(const locale<locale_code, encoding>&) { return TFacet(); }
//template <class TFacet, class TLocale>
//inline TFacet use_facet(TLocale) { return TFacet(); }

/*
 * TODO: Try use_facet_ctype, which is not standard, but may be the best we can do without going
 * full trailing/auto return type
template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
inline ctype<locale_code, encoding, TChar> use_facet2<ctype_test<TChar>>(const locale<locale_code, encoding>&)
{
    return ctype<locale_code, encoding, TChar>();
} */

template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
inline ctype<locale_code, encoding, TChar> use_facet_ctype(const locale<locale_code, encoding>&)
{
    return ctype<locale_code, encoding, TChar>();
}

template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
struct use_facet_helper<ctype_test<TChar>, locale_code, encoding>
{
    typedef ctype<locale_code, encoding, TChar> value_type;

    static value_type use_facet(const locale<locale_code, encoding>&)
    {
        return value_type();
    }
};

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
inline typename use_facet_helper<TFacet, locale_code, encoding>::value_type
    use_facet3(const locale<locale_code, encoding>& loc)
{
    typedef use_facet_helper<TFacet, locale_code, encoding> helper_type;

    return helper_type::use_facet(loc);
}

/*
template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
inline ctype<locale_code, encoding, TChar> use_facet3(const locale<locale_code, encoding>& loc)
{
    //typedef use_facet_helper<ctype_test<TChar>> helper_type;

    //helper_type::use_facet(loc);
    return ctype<locale_code, encoding, TChar>();
} */


template <locale_code_enum locale_code, internal::encodings::values encoding, class TChar>
inline bool isspace(TChar ch, const locale<locale_code, encoding>& loc)
{
    typedef ctype<locale_code, encoding, TChar> ctype_type;
    return use_facet<ctype_type>(loc).is(ctype_base::space, ch);
}



//classic_locale classic();

}}
