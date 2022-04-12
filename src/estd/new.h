#pragma once

#ifdef FEATURE_STD_NEW
#include <new>
#else
inline void* operator new(unsigned, void* p) {return p; }
inline void* operator new[](unsigned, void *p) { return p; }
#endif