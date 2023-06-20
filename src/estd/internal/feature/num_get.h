#pragma once

// As per
// https://stackoverflow.com/questions/17116690/c-read-int-from-istream-detect-overflow
// https://cplusplus.github.io/LWG/issue23
// Enables, during istream/num_get int parsing, additional overwriting of
// incoming integer to reflect 0, min or max under various error conditions
// DEBT: Improve feature name
#ifndef FEATURE_ESTD_NUM_GET_LWG23
#define FEATURE_ESTD_NUM_GET_LWG23 1
#endif
