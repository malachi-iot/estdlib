#pragma once

#include "fwd.h"
#include "cbase.h"


namespace estd {

// In development, not ready, so marked as internal
namespace internal {

template <class TChar, class OutputIt = void, class TLocale = void>
class num_put
{
public:
    typedef TChar char_type;
    typedef OutputIt iter_type;

    template <class T>
    static iter_type get(iter_type out,
        const ios_base& str, ios_base::iostate& err,
        T& v)
    {
        return {};
    }
};

}
}