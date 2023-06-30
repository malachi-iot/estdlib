#pragma once

#include "cstdint.h"

#include "internal/platform.h"
#include "internal/locale.h"

#include "internal/iosfwd.h"

#include "internal/locale/ctype.h"
#include "internal/locale/facet.h"
#include "internal/locale/money.h"
#include "internal/locale/num_get.h"
#include "internal/locale/num_put.h"
#include "internal/locale/numpunct.h"


namespace estd { namespace experimental {


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


struct locale : internal::locale_base_base
{
    // Maps closely to the GNU interpretation/combination of
    // ISO 639-1 and ISO 3166 (i.e. en_US)
    // https://www.gnu.org/software/gettext/manual/html_node/Usual-Language-Codes.html
    // https://www.gnu.org/software/gettext/manual/html_node/Country-Codes.html
    typedef internal::locale_code::values iso;
    typedef internal::encodings::values encodings;

#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <iso iso_code, encodings encoding>
    using type = internal::locale<iso_code, encoding>;
#else
    // FIX: We have to find a way to inherited class specialization
    // to work before this itself can work.  PGGCC-25
    //template <iso iso_code, encodings encoding>
    //struct type : internal::locale<iso_code, encoding> {};
#endif

    typedef internal::classic_locale_type classic_type;
    // DEBT: c++03 doesn't like below line.  Find out why
    //typedef internal::locale<iso::C, encodings::ASCII> classic_type;

    inline static classic_type classic() { return classic_type(); }
};

template <class TChar, class TLocale>
inline bool isspace(TChar ch, const TLocale& loc)
{
    return use_facet<ctype<TChar> >(loc).is(ctype_base::space, ch);
}

}
