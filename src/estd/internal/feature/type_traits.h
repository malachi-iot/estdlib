#pragma once

#include "../platform.h"

// By default, if available, we alias a bunch of type_traits stuff to its underlying
// std.  This can be turned off to use a subset of our own implementation which is
// an implicit reality in environments such as AVR which don't have type_traits available
//#if __cpp_alias_templates && FEATURE_STD_TYPE_TRAITS  // In all cases, we need __cpp_alias_template
#ifndef FEATURE_ESTD_TYPE_TRAITS_ALIASED
#define FEATURE_ESTD_TYPE_TRAITS_ALIASED FEATURE_STD_TYPE_TRAITS
#endif
