#pragma once

#include "../feature/functional.h"
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

template<typename T>
struct function_traits;

namespace detail {

namespace impl {

template <typename F>
using function_default = function_fnptr2<F>;

}

inline namespace v1 {
template <typename F, class Impl = impl::function_default<F> >
class function;
}

namespace v2 {
template <typename F, template <class> class Impl = impl::function_default >
using function = detail::v1::function<F, Impl<F> >;
}

}

namespace internal {

struct function_base_tag {};

template <typename F, template <class> class Impl = detail::impl::function_fnptr1>
class thisify_function;

#ifdef __cpp_variadic_templates
template <typename F, typename ...TContexts>
class contextify_function;
#endif

}

}
