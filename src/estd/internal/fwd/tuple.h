#pragma once

#include "tuple-shared.h"

namespace estd {

#if __cpp_variadic_templates
template<class... TArgs>
class tuple;
#endif

}
