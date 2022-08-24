#pragma once

namespace estd {

template <class T>
struct hash;

namespace detail {

namespace impl {

template <typename F>
struct function_fnptr1;

template <typename F>
struct function_fnptr2;

template <typename F>
struct function_virtual;

template <typename F>
using function_default = function_fnptr1<F>;

}

template <typename F, class TImpl = impl::function_default<F> >
class function;

}

namespace internal {

struct function_base_tag {};

template <typename F>
class context_function;

}

}
