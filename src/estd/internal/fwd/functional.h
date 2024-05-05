#pragma once

#include "../impl/functional/fwd.h"

namespace estd {

// Deviates from normal hash with 'precision' - somewhat experimental
// DEBT: default Precision to estd::size_t
template <class Key, class Precision = unsigned>
struct hash;

template <class T = void>
struct equal_to;

template <class T = void>
struct less;

template <class T = void>
struct greater;

namespace detail {

namespace impl {

#ifdef __cpp_alias_templates
template <typename F>
using function_default = function_fnptr2<F>;
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
