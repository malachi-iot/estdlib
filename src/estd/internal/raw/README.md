# Implementations

Called *raw* instead of *implementation* do disambiguate from nearby `impl`
folder

Place where implementation of certain headers go, sans predicating 
`#include` for platform discovery, etc. 

We need this for a small number of internal support headers which themselves
are called by platform discovery and thus can't call the main estd/xxx.h
since it would #pragma filter it away