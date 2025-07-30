#pragma once

#include "../feature/functional.h"
#include "../impl/functional/fwd.h"

namespace estd {

template <class Key>
struct hash;

template <class T = void>
struct equal_to;

template <class T = void>
struct not_equal_to;

template <class T = void>
struct less;

template <class T = void>
struct greater;

template<typename T>
struct function_traits;

namespace detail {

namespace impl {

template <typename F, impl::fn_options o = impl::FN_DEFAULT>
using function_default = function_fnptr2<F, o>;

}

inline namespace v1 {
template <typename Signature, class Impl = impl::function_default<Signature> >
class function;
}

namespace v2 {
template <typename Signature, template <class, impl::fn_options> class Impl = impl::function_default,
    impl::fn_options o = impl::FN_DEFAULT>
using function = detail::v1::function<Signature, Impl<Signature, o> >;
}

}

namespace internal {

struct function_base_tag {};

template <typename F, template <class, detail::impl::fn_options> class Impl = detail::impl::function_fnptr1>
class thisify_function;

#ifdef __cpp_variadic_templates
template <typename F, typename ...TContexts>
class contextify_function;
#endif

}

}
