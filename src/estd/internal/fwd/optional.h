#pragma once

namespace estd {

namespace internal {

template <class T>
struct optional_base;

// EXPERIMENTAL
// I don't love that it's encouraged to have '0' as default null value integer, so
// playing with this.
template <class T, class enabled = void>
struct optional_default_value;

}

template <class T, class TBase = internal::optional_base<T> >
class optional;

}