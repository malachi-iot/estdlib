#pragma once

#ifndef ESTD_UNORDERED_MAP_BUCKET_SIZE
#define ESTD_UNORDERED_MAP_BUCKET_SIZE 4
#endif

// Perform additional sanity checks along the way
#ifndef ESTD_UNORDERED_MAP_STRICT
#define ESTD_UNORDERED_MAP_STRICT 1
#endif

// If you want fine grained control of how Nullable works, this enables
// 'Nullable' as a template parameter for 'unordered_traits'.  Disabled
// by default because it increases error log bulk and is rarely needed
// (i.e. estd::optional or specialization covers most edge cases)
#ifndef ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
#define ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE 0
#endif

#ifndef ISSUE_211_BRINGUP
#define ISSUE_211_BRINGUP 0
#endif

// EXPERIMENTAL
// This approach reserves one null slot so that tombstone -> null conversion
// has a foothold in reality.  See null_slots() method for more info
#ifndef ESTD_UNORDERED_MAP_RESERVE_SLOT_OPT
#define ESTD_UNORDERED_MAP_RESERVE_SLOT_OPT 1
#endif
