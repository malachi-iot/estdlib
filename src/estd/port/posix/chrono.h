#pragma once

// DEBT: This isn't really a posix chrono, this is alias to "real" std::chrono, so move to std area
// mainly serves as aliases to std chrono

#include <estd/chrono.h>

#ifdef FEATURE_STD_CHRONO

namespace estd {

namespace chrono {

// NOTE: Probably we should reimplement this so that 'now()' returns
// our own duration.  Either that or swing the other way and when in
// POSIX mode map everything over to std::chrono.  The only trouble
// with that is then we don't test our own lib version
typedef std::chrono::steady_clock steady_clock;

}

}

#endif
