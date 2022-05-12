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

template <locale_code_enum locale_code, internal::encodings::values encoding, class TChar>
class ctype : public ctype_base
{
#ifdef ENABLE_LOCALE_MULTI
    // TODO: determine if we want to roll with the virtual function do_is
    // and friends or branch out into further templating
#else
    TChar do_tolower(TChar ch);
    TChar do_toupper(TChar ch);
#endif
public:
    bool is(mask m, TChar ch) const { return false; }
    const TChar* is(const TChar* low, const TChar* high, mask* vec) const { return NULLPTR; }

    TChar toupper(TChar ch) { return do_toupper(ch); }
    TChar tolower(TChar ch) { return do_tolower(ch); }
};

template <locale_code::values locale_code, internal::encodings::values encoding>
struct locale
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

    typedef int category;

    static CONSTEXPR category none = 0x0000;
    static CONSTEXPR category ctype = 0x0001;
    static CONSTEXPR category numeric = 0x0002;

    // TODO: deviates in that standard version uses a std::string
    // I want my own std::string (beginnings of which are in experimental::layer3::string)
    // but does memory allocation out of our own GC-pool
    const char* name() const { return locale_name<locale_code, encoding>(); }
};


// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <locale_code_enum locale_code>
class ctype<locale_code, estd::internal::encodings::ASCII, char> :
    public ctype_base,
    public locale<locale_code, estd::internal::encodings::ASCII>::facet
{
public:
    typedef char char_type;

    //static locale::id id;

    char widen(char c) const { return c; }

    bool is(mask m, char ch) const
    {
        if(m & space)
        {
            // as per http://en.cppreference.com/w/cpp/string/byte/isspace
            switch(ch)
            {
                case ' ':
                case 13:
                case 10:
                case '\f':
                case '\t':
                case '\v':
                    return true;
            }
        }
        if(m & digit)
        {
            if(ch >= '0' && ch <= '9') return true;
        }
        return false;
    }
};

template <locale_code_enum locale_code>
class ctype<locale_code, estd::internal::encodings::UTF8, char> :
    public ctype<locale_code, estd::internal::encodings::ASCII, char> {};


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




// DEBT: Consolidate all this with char_base_traits

/*
 * Can't remember if this is possible / how to do it
template <class TChar, class InputIt>
template <>
inline typename num_get<TChar, InputIt>::iter_type num_get<TChar, InputIt>::get<long>(
    iter_type in,
    iter_type end, estd::ios_base::iostate& err, long& v) const
{
    return in;
}
 */

}}
