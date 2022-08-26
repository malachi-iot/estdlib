

## FEATURE_ESTD_AGGRESIVE_BITFIELD

Use bit packed structs to sacrifice speed for space

## FEATURE_ESTD_ALGORITHM_OPT

Normally estd aliases out some of the <algorithm> functions in hopes that
your particular platform has a nice optimized version.  This can be disabled
by setting FEATURE_ESTD_ALGORITHM_OPT to 0

## FEATURE_ESTD_FREERTOS_THREAD

Enable or disable estd::mutex and similar wrappers for freertos.
Defaults to true.  Note that estd::freertos::mutex and similar are
enabled always.

## FEATURE_ESTD_IOS_GCOUNT

Enable or disable 'gcount' feature in istream
