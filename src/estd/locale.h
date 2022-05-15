#pragma once

#include "internal/platform.h"
#include "internal/locale.h"

#include "internal/iosfwd.h"

#include "internal/locale/ctype.h"
#include "internal/locale/facet.h"
#include "internal/locale/money.h"
#include "internal/locale/num_get.h"
#include "internal/locale/numpunct.h"

extern "C" {
#include <stdint.h>
}

namespace estd { namespace experimental {

#if UNUSED
// DEBT: Need to move this out to regular estd and rename to 'locale'
// so that calls like 'classic' have a std-like signature
// NOTE: Due to how we specialize, this class cannot be base class of our specialized locale
struct locale_base : locale_base_base
{
};
#endif





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



}

namespace internal {

template <internal::locale_code::values locale_code, internal::encodings::values encoding>
struct locale : locale_base_base
{
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
    const char* name() const { return internal::locale_name<locale_code, encoding>(); }
};

}

struct locale : internal::locale_base_base
{
    // Maps closely to the GNU interpretation/combination of
    // ISO 639-1 and ISO 3166 (i.e. en_US)
    // https://www.gnu.org/software/gettext/manual/html_node/Usual-Language-Codes.html
    // https://www.gnu.org/software/gettext/manual/html_node/Country-Codes.html
    typedef internal::locale_code::values iso;
    typedef internal::encodings::values encodings;

    template <iso iso_code, encodings encoding>
    using type = internal::locale<iso_code, encoding>;
    /*
    struct text : internal::locale<iso_code, encoding> {}; */

    typedef internal::locale<iso::C, encodings::ASCII> classic_type;

    inline static classic_type classic() { return classic_type(); }
};

template <class TChar, class TLocale>
inline bool isspace(TChar ch, const TLocale& loc)
{
    return use_facet<ctype<TChar>>(loc).is(ctype_base::space, ch);
}

}
