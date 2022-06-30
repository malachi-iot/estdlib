/**
 * @file
 * More complex platform support goes here, called at the end of the
 * platform.h deduction chain and benefits from all its discovery
 * As this is an internal .H call, we don't do a predicating 
 * #include "platform.h" here
 */
// Re-evaluating this over and over since it does appear rather early
// in the #include chain and support code may need additional dependencies
// brought on later
//#pragma once

#ifdef ESTD_ARDUINO
#include "arduino/Print.h"
#endif
