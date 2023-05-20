#ifdef _MSC_BUILD
#define CATCH_CONFIG_RUNNER
#else
#define CATCH_CONFIG_MAIN
#endif

#include <catch.hpp>

#include "test-data.h"

namespace estd { namespace test {

unsigned NonTrivial::dtor_counter = 0;

}}
