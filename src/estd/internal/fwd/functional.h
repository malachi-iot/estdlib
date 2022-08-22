#pragma once

namespace estd {

template <class T>
struct hash;

namespace internal {

struct function_base_tag {};

}

namespace detail {

namespace impl {

template <typename T>
struct function_fnptr1;

template <typename T>
struct function_fnptr2;

template <typename T>
struct function_virtual;

}

template <typename T, class TImpl = impl::function_fnptr1<T> >
class function;

}

}