#pragma once

#include "../../../flags.h"

namespace estd { namespace detail { namespace impl {

enum fn_options
{
    FN_NONE,

    // +++ 03APR26 MB DEBT: Deprecated - seems more sensible to have discrete
    // function_xxx impls for each interesting condition.  Keeping fn_options overall
    // since we've invested in it and we may want some other option at some point
    FN_COPY,            // fn(const fn&)
    FN_MOVE,            // fn(fn&&)
    FN_DTOR,            // ~fn
    FN_AUTO_DTOR,       // ~fn *always* after execution (opt mode)
    // ---

    // TODO: Consider fnptr1, fnptr2 and virtual specifiers here too

    FN_DEFAULT = FN_NONE,
};

ESTD_FLAGS(fn_options)

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr1;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr1_oneshot;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr2;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr2_trivial;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr2_oneshot;

template <typename F, fn_options = FN_DEFAULT>
struct function_virtual;

template <typename F, fn_options o = FN_DEFAULT>
using function_fnptr2_opt = function_fnptr2_oneshot<F, o>;

// 23NOV25 MB In progress, coming along:
// 1. fnptr1 no #135 implementation
// 2. fnptr2 #135 implementation, passes basic testing
// 3. virtual #135 implementation, passes basic testing
#ifndef FEATURE_ESTD_GH135
#define FEATURE_ESTD_GH135 1
#endif

// EXPERIMENTAL
// 23NOV25 MB - Does actually work.  I am concerned that extra references
// in place of copies may actually be *more* overhead.  Idea would be to detect
// whether we want regular Args..., Args&&... or a container to pass args around.
// That's a deep dive optimization.  Probably worth it at some point, but not
// today
#ifndef FEATURE_ESTD_FUNCTION_RVALUE
#define FEATURE_ESTD_FUNCTION_RVALUE 0
#endif

}}}

namespace estd { namespace internal {

template <class R, class ...Args>
struct function_verify_args_match;

}}
