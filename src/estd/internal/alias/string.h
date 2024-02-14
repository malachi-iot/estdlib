#pragma once

#include "../fwd/string.h"

// Aggressively aliases std namespace to estd
// In support of https://github.com/malachi-iot/estdlib/issues/23 dragonbox

#if FEATURE_ESTD_STD_ALIAS

// TODO: Do memcpy here

namespace std {
    
}

#endif