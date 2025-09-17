#pragma once

#include "base.h"

namespace estd { namespace internal {

template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::counter>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

protected:
    using base_type::atomic;

    using iterator = typename base_type::pos_iterator;

#if FEATURE_STD_ATOMIC
    using counter_type = conditional_t<atomic, std::atomic<unsigned>, unsigned>;
#else
    using counter_type = unsigned;
#endif

    counter_type size_{};

    ESTD_CPP_CONSTEXPR(14) void decrement_size()
    {
        --size_;
    }

    ESTD_CPP_CONSTEXPR(14) void increment_size()
    {
        ++size_;
    }

    constexpr circular_queue_base() = default;

    template <class ...Args>
    constexpr explicit circular_queue_base(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

    ESTD_CPP_CONSTEXPR(14) void clear_size()
    {
        size_ = 0;
    }

public:
    using typename base_type::size_type;

    constexpr bool empty() const { return size_ == 0; }
    constexpr size_type size() const { return size_; }
};


}}
