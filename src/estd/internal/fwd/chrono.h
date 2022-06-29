#pragma once

namespace estd { namespace chrono {

// DEBT: Would do estd::chrono::internal, but that collides with our estd_chrono trick in port area
namespace internal {

// To be applied to traits specifically
struct unix_epoch_clock_tag {};

// Since C++20 epoch is midnight 1/1/1970 GMT, and is loosely considered standardized on that
// prior to C++20.  I haven't found any API in the spec which actually reflects that though, so
// we make our own.
// DEBT: Just found a very similar one, https://en.cppreference.com/w/cpp/chrono/clock_time_conversion
template <class TClock>
struct clock_traits;


// embedded-oriented version has lower precision.  Deviates from standard
// DEBT: Revisit this on a per-platform and option-selectable level
#ifdef FEATURE_ESTD_CHRONO_LOWPRECISION
typedef int32_t nano_rep;
typedef int32_t micro_rep;
typedef int32_t milli_rep;
typedef int16_t seconds_rep;
typedef int16_t minutes_rep;
typedef int16_t hours_rep;
typedef int16_t days_rep;
typedef int8_t weeks_rep;
typedef int8_t months_rep;
typedef int8_t years_rep;
#else
typedef int64_t nano_rep;
typedef int64_t micro_rep;
typedef int64_t milli_rep;
typedef int64_t seconds_rep;
typedef int32_t minutes_rep;
typedef int32_t hours_rep;
typedef int32_t days_rep;
typedef int32_t weeks_rep;
typedef int32_t months_rep;
typedef int16_t years_rep;      ///< Deviates from spec which calls for 17 bit minimum
#endif


}

}}