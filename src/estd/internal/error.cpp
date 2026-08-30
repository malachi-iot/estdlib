#include "error_code.h"

namespace estd { namespace internal {

#define CASE(x) case errc::x: return #x;

template <>
template <>
const char* error_traits<errc, generic_category_tag>::to_string<classic_locale_type>(errc err)
{
    switch(err)
    {
        CASE(address_family_not_supported)
        CASE(address_in_use)
        CASE(bad_address)
        CASE(connection_refused)
        CASE(file_exists)
        CASE(host_unreachable)
        CASE(invalid_argument)
        CASE(io_error)
        CASE(message_size)
        CASE(no_space_on_device)
        CASE(no_message)
        CASE(not_enough_memory)
        CASE(resource_unavailable_try_again)
        CASE(result_out_of_range)
        CASE(timed_out)
        CASE(value_too_large)
        default: return "N/A";
    }
}

}}
