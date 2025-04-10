#pragma once

// As per https://www.ietf.org/rfc/rfc2119.txt

namespace estd { namespace internal {

inline namespace rfc {

enum class rfc2119
{
    must_not,
    shall_not = must_not,
    should_not,
    not_recommended = should_not,
    may,
    optional = may,
    should,
    recommended = should,
    must,
    required = must,
    shall = must,
};

using terms = rfc2119;

}

}}
