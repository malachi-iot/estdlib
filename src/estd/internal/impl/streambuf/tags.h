#pragma once

// because ESTD_FN_HAS_METHOD was a bust, trying out explicit tagging instead

namespace estd { namespace internal {

// Indicates streambuf impl provides its own sbumpc.  This is useful in
// scnearios when wrapping another API which itself provides a 'getch' and
// not a lot of buffer access.
struct streambuf_sbumpc_tag {};

// Indicates streambuf impl provides gptr and egptr
struct streambuf_gptr_tag {};

// Indicates streambuf impl provides sungetc
struct streambuf_sungetc_tag {};

// Indicates streambuf impl provides showmanyc
struct streambuf_showmanyc_tag {};

// Indicates streambuf impl supports blocking
// NOT YET USED
struct streambuf_blocking_tag {};

}}