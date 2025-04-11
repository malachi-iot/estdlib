/**
 * @file
 * NOTE: data() implementation sticks to <= c++11 form in which does not need to be null terminated
 */
#pragma once

#include "memory.h"
#include "allocators/fixed.h"
#include "traits/char_traits.h"
#include "traits/string.h"
#include "internal/string.h"
#include "internal/fwd/string_view.h"
#include "internal/to_string.h"
#include "policy/string.h"
#include "port/string.h"
#include "algorithm.h"
#include "span.h"
#include "cstdlib.h"

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
//#include <ostream>
#endif

#include "internal/macro/push.h"

namespace estd {

#ifdef FEATURE_STD_MEMORY
using string = basic_string<char>;
#endif


// DEBT: Move this out to layer3/string.h if we plan to keep const_string.
// That is difficult because it's template-instantiating basic_string
namespace layer3 {

// Non-NULL-terminated const strings use layer3
// NULL-terminated const strings use layer2
// NOTE: Remake this into basic_string_view.  Though the basic_string_view has an additional variable (potentially)
//       for tracking the truncations
// If the above does not happen, then consider promoting/typedefing this const_string to the regular
// estd:: namespace too
class const_string : public basic_string<const char, false>
{
    typedef basic_string<const char, false> base_t;
    typedef base_t::size_type size_type;

public:
    const_string(const char* s, size_type len) :
        base_t(s, len, true) {}


    template <size_type N>
    const_string(const char (&buffer) [N], bool source_null_terminated = true) :
        base_t(buffer, source_null_terminated ? strlen(buffer) : N, true) {}

    // convenience method since std::vector and std::string itself are reported to convert
    // uneventfully between unsigned char and char
    // note it's a little bit bad because of the distant possibility of a
    // byte != unsigned char != uint8_t
    const_string(const estd::span<const uint8_t>& cast_from) :
        base_t(reinterpret_cast<const char*>(cast_from.data()), cast_from.size(), true) {}
};

}


// FIX: This doesn't account for conversion errors, but should.  std version
// throws exceptions
// DEBT: Rework to use num_get/from_string - doing so will make it easier
// to handle non-null terminated strings
template <class Impl>
long stol(
        const detail::basic_string<Impl>& str,
        size_t* pos = 0, int base = 10)
{
    // FIX: very clunky way to ensure we're looking at a null terminated string
    // policy won't have this method otherwise
    Impl::policy_type::is_null_termination(0);

    typedef typename Impl::policy_type::char_traits::char_type char_type;

    const char_type* data = str.clock();
    char_type* end;
    long result = strtol(data, &end, base);
    str.cunlock();
    if(pos != NULLPTR)
        *pos = end - data;

    return result;
}

template <class Impl>
unsigned long stoul(
        const detail::basic_string<Impl>& str,
        size_t* pos = 0, int base = 10
        )
{
    using string_type = detail::basic_string<Impl>;

    // FIX: very clunky way to ensure we're looking at a null terminated string
    // somehow I never expose null_termination indicators on the string itself
    //TStringTraits::is_null_termination(0);
    string_type::policy_type::is_null_termination(0);
    using char_type = typename string_type::value_type;

    const char_type* data = str.clock();
    typename estd::remove_const<char_type>::type* end;
    unsigned long result = strtoul(data, &end, base);
    str.cunlock();
    if(pos != NULLPTR)
        *pos = end - data;

    return result;
}


// TODO: Utilized optimized version for int precision only
// NOTE: Interestingly, spec calls for stoi to call strtol or wcstol.
// TODO: This is  an optimization opportunity, to call something like atoi
// for lower-precision conversion
template <class Impl>
int stoi(
        const detail::basic_string<Impl>& str,
        size_t* pos = 0, int base = 10)
{
    return (int) stol(str, pos, base);
}


}


#include "internal/macro/pop.h"




