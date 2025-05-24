#pragma once

// std::string auto initializes itself to empty
// we followed suit, but on further reflection default uninitialized is more suitable for
// embedded space.  This would be a breaking change, so we've feature flagged it
// https://github.com/malachi-iot/estdlib/issues/115
// Flag ignored at this time (defautls to always value-initalizing string)
#ifndef FEATURE_ESTD_STRING_DEFAULT_VALUE_INIT
#define FEATURE_ESTD_STRING_DEFAULT_VALUE_INIT 1
#endif

// DEBT: Needs better doc
// Only applies to shifted_string usage in to_string at the moment
#define FEATURE_ESTD_TO_STRING_OPT 1
