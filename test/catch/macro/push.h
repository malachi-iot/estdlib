// DEBT: Might be better to do all this at cmake level.  In case we want to undef
// some things, this still could be the best way

#if __GNUC__
#ifndef __has_warning
// GNUC is permissive of disabling warnings even if it doesn't know about them, I think
#define __has_warning(x)    (true) // FAKE has warning
#endif
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#if __has_warning("-Wunused-but-set-variable")
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
