#pragma once

// Passes unit tests, needs just a bit more inspection and
// should be good to go - after we un-hard-wire the null termination, that is
#define FEATURE_ESTD_STRICT_DYNAMIC_ARRAY

// legacy option, when we were heading towards allocators participating in how many
// times a handle has been locked (pinned).  Seems it's better to do this externally,
// so unless you have something specific in mind, keep this disabled
//#define FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER

// experimental feature to use proper strong-typed handles more often vs resoloving to
// 'just a bool' - doesn't work yet, some abiguity on lock overload fails it
//#define FEATURE_ESTD_ENHANCED_HANDLE_EXP
