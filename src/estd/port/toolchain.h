#pragma once

// TODO: Put in CLang handler also

#if defined(__GNUC__)
#include "../port/toolchain/gnuc.h"
#endif

#if defined(_MSC_VER)
#include "../port/toolchain/msvc.h"
#endif

// NOTE: May just be identical to gnuc, but rolling distinct for now just incase
// we wanna add something
#if defined(__ADSPBLACKFIN__)
#include "../port/toolchain/visualdsp.h"
#endif

