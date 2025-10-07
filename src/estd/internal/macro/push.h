// Arduino sets a bunch of these
#ifdef FEATURE_PRAGMA_PUSH_MACRO
#pragma push_macro("_abs")  // ESP32 specific
#pragma push_macro("abs")
#pragma push_macro("max")
#pragma push_macro("min")
#pragma push_macro("round")
#undef _abs
#undef abs
#undef max
#undef min
#undef round
#endif
