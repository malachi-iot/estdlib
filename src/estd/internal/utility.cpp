#include "platform.h"
#include "utility.h"

#ifndef FEATURE_CPP_INLINE_STATIC
namespace estd { namespace internal {

CONSTEXPR has_member_base::yes has_member_base::yes_value = { 0 };
CONSTEXPR has_member_base::no has_member_base::no_value = { { 0 }, { 1 } };

}}
#endif
