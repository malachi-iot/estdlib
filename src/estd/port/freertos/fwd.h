#include "../../internal/platform.h"

#pragma once

namespace estd { namespace freertos {

template <bool static_allocated>
class mutex;

template <bool static_allocated>
class recursive_mutex;

template <unsigned max, bool static_allocated>
struct counting_semaphore;

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <bool static_allocated>
using binary_semaphore = counting_semaphore<1, static_allocated>;
#else
template <bool static_allocated>
class binary_semaphore;
#endif

template <class T, bool static_allocated = false>
class queue;

}}