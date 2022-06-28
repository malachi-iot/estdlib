#pragma once

namespace estd {

// DEBT: Would do estd::chrono::internal, but that collides with our estd_chrono trick in port area
namespace internal {

template <class TClock>
struct clock_traits;

}

}