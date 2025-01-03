#pragma once

#define FEATURE_ESTD_BYTESWAP_AUTO  0       // Prefer STD, then GCC (if available), then XOR
#define FEATURE_ESTD_BYTESWAP_STD   1       // Force STD mode, ignoring compiler indication of availability
#define FEATURE_ESTD_BYTESWAP_GCC   2       // Force GCC mode, ignoring compiler indication of availability
#define FEATURE_ESTD_BYTESWAP_XOR   3       // Force XOR (manual) mode, not implemented at this time

#ifndef FEATURE_ESTD_BYTESWAP_MODE
#define FEATURE_ESTD_BYTESWAP_MODE  FEATURE_ESTD_BYTESWAP_AUTO
#endif
