#pragma once

// alpha-level feature to expand meaning of streambuf's traits_type to include streambuf characteristics,
// not just char_traits.  This is specifically used in support of cts/dtr style signaling as
// per https://github.com/malachi-iot/estdlib/issues/29
// Making good progress, disabled for now since until it gets really beefy, it's not so different
// from running a streambuf in blocking mode.
#ifndef FEATURE_ESTD_STREAMBUF_TRAITS
//#define FEATURE_ESTD_STREAMBUF_TRAITS __cpp_constexpr
#endif