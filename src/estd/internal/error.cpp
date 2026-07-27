#include "error_code.h"

namespace estd { namespace internal {

#define CASE(x) case errc::x: return #x;

template <>
template <>
const char* error_traits<errc>::to_string<system_category_tag>(errc err)
{
    switch(err)
    {
        CASE(address_family_not_supported)
        CASE(invalid_argument)
        CASE(not_enough_memory)
        CASE(resource_unavailable_try_again)
        CASE(result_out_of_range)
        default: return "N/A";
    }
}

}}