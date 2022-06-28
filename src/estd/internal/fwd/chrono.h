#pragma once

namespace estd {

// DEBT: Would do estd::chrono::internal, but that collides with our estd_chrono trick in port area
namespace internal {

// To be applied to traits specifically
struct unix_epoch_clock_tag {};

// Since C++20 epoch is midnight 1/1/1970 GMT, and is loosely considered standardized on that
// prior to C++20.  I haven't found any API in the spec which actually reflects that though, so
// we make our own
template <class TClock>
struct clock_traits;

}

}