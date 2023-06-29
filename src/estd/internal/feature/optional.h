#pragma once

#include "exception.h"

// estd::layer1::optional in c++ defaults to a template alias, while c++03
// has to make do with a chunky wrapper class.  By default this feature
// flag enables the template alias.  Set to 0 to force into c++03 non-alias
// mode.
#if __cpp_alias_templates
#ifndef FEATURE_ESTD_OPTIONAL_LAYER1_ALIAS
#define FEATURE_ESTD_OPTIONAL_LAYER1_ALIAS 1
#endif
#endif