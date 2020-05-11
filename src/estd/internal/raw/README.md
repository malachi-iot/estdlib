# Implementations

Called *raw* instead of *implementation* do disambiguate from nearby `impl`
folder

Do not include these explicitly unless you know what you're doing

Place where implementation of certain headers go, sans predicating 
`#include` for platform discovery, etc.  These *raw* headers may
include each other

We need this for a small number of internal support headers which themselves
are called by platform discovery and thus can't call the main estd/xxx.h
since it would #pragma filter it away

## TODO: Consider `#include "platform.h"` in each

So that tooltips etc resolve more.  #pragmas will keep it safe 