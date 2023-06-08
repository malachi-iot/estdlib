#pragma once

#include "../fwd/variant.h"
#include "../raw/utility.h"
#include "../raw/variant.h"
#include "../feature/variant.h"

#include "../variadic.h"

#include "../../new.h"

#if __cpp_exceptions
#include <exception>
#endif

namespace estd {

#if __cpp_exceptions
class bad_variant_access : std::exception
{
public:
};
#endif

}
