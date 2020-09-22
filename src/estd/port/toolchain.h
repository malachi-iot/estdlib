#pragma once

#include "toolchain/gnuc-flavor.h"

// TODO: Put in CLang handler also

#if defined(__GNUC__)
#include "toolchain/gnuc.h"
#endif

#if defined(_MSC_VER)
#include "toolchain/msvc.h"
#endif

// NOTE: May just be identical to gnuc, but rolling distinct for now just incase
// we wanna add something
#if defined(__ADSPBLACKFIN__)
#include "toolchain/visualdsp.h"
#endif
