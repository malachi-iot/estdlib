#pragma once

namespace estd {

namespace detail {

// For our non-standard << overload which takes a functor
struct ostream_functor_tag {};

}

// TODO: use specific 16/32/64 bit versions depending on architecture
typedef int streampos;
typedef int streamoff;
typedef int streamsize;

}