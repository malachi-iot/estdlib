#pragma once

// As per https://www.ietf.org/rfc/rfc2119.txt

namespace estd { namespace internal {

enum class rfc2119
{
    must,
    required = must,
    shall = must,
    should,
    recommended = should,
    may,
    optional = may,
    should_not,
    not_recommended = should_not,
    must_not,
    shall_not = must_not
};

}}
