#pragma once

#include "../traits/string.h"

// 24MAY25 MB - Just about ready to be a 'detail' rather than 'internal', just playing it
// conservatively for now.  Perhaps estd::policy namespace ought to be considered
namespace estd { namespace internal {

// explicit constant specified here because:
// - char_traits by convention doesn't specify const
// - even if it did, this policy applies to non string allocated arrays too
template <class Size, bool constant_>
struct buffer_policy :
    policy_base,    // EXPERIMENTAL, not used directly
    experimental::is_const_tag_exp_base<constant_>
{
    using size_type = Size;

    static ESTD_CPP_CONSTEVAL bool is_constant() { return constant_; }
};

// DEBT: Refactor string_policy to take a null_terminated flag directly, then specialize
template <class CharTraits, class Size, string_options o, class Enabled>
struct string_policy :
//struct string_policy<CharTraits, Size, o,
//    estd::enable_if_t<o & string_options::null_terminated>> :
    buffer_policy<Size, o & string_options::constant>
{
    using char_traits = CharTraits;
};

// Denotes a string whose size is tracked via traditional C null termination
template <class CharTraits, class Size = size_t, bool constant = false>
struct null_terminated_string_policy : string_policy<CharTraits, Size, constant ? string_options::constant : string_options::none>
{
    typedef void is_null_terminated_exp_tag;
#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR bool is_null_terminated() { return true; }
#endif

    static CONSTEXPR bool is_null_termination(const char& value) { return value == 0; }
};


// Denotes a string whose buffer size is tracked at runtime via an integer
template <class CharTraits, class Size, bool constant>
struct sized_string_policy  : string_policy<CharTraits, Size, constant ? string_options::constant : string_options::none>
{
    // NOTE: As of this writing, this tag is not used
    typedef void is_explicitly_sized_tag_exp;

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR bool is_null_terminated() { return false; }
#endif
};

}}


