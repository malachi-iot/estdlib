#pragma once

//#include "../platform.h"

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

#ifdef __cpp_alias_templates
template <typename F>
using function_default = function_fnptr1<F>;
#endif

}

#ifdef __cpp_alias_templates
template <typename F, class TImpl = impl::function_default<F> >
#else
template <typename F, class TImpl = impl::function_fnptr1<F> >
#endif
class function;

}

namespace internal {

struct function_base_tag {};

template <typename F>
class thisify_function;

#ifdef __cpp_variadic_templates
template <typename F, typename ...TContexts>
class contextify_function;
#endif

}

}
