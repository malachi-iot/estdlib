#pragma once

#include "gnuc.h"

// Using https://sourceforge.net/p/predef/wiki/DataModels/ as reference as well
// as "VisualDSP++ 4.5 C/C++ Compiler and Library Manual" Table 1-35
// DEBT: Isolate this to Blackfin only
#ifndef __ILP32__
#define __ILP32__
#endif
