#pragma once

#include "fwd.h"

namespace estd { namespace internal {

// This guy won't inherit from circular_queue_container_base, though he shares
// the container, front/back are managed differently
template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::packed>>
{
    // TODO: Since one would only used this packed flavor in a super constrained environment,
    // consider an EBO approach where a global pointer is used/no pointer (in this) is tracked

    static constexpr unsigned word_bits = sizeof(unsigned) * 8;
    static constexpr unsigned index_bits = (word_bits >> 1) - 1;

    struct
    {
        unsigned front_ : index_bits;
        unsigned back_ : index_bits;
        bool empty_ : 1;
    };

public:
    constexpr circular_queue_base() : front_{}, back_{}, empty_{true} {}
};

}}
