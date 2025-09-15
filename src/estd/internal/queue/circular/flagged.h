#pragma once

#include "base.h"

namespace estd { namespace internal {

template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::flagged>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

protected:
    using base_type::atomic;
    using base_type::array_;
    using base_type::front_;
    using base_type::back_;

    using iterator_base = typename base_type::pos_iterator_base;

#if FEATURE_STD_ATOMIC
    using bool_type = conditional_t<atomic, std::atomic_flag, bool>;
#else
    using bool_type = bool;
#endif

    bool_type empty_{true};

    ESTD_CPP_CONSTEXPR(14) void increment_size()
    {
        empty_ = false;
    }

    ESTD_CPP_CONSTEXPR(14) void decrement_size()
    {
        empty_ = back_ == front_;
    }

    ESTD_CPP_CONSTEXPR(14) void clear_size()
    {
        empty_ = true;
    }

    constexpr circular_queue_base() = default;

    template <class ...Args>
    constexpr explicit circular_queue_base(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

public:
    constexpr bool empty() const { return empty_; }
    using typename base_type::size_type;

    size_type size() const
    {
        if(empty_) return 0;

        if(front_ >= back_)
            return array_.size() - (front_ - back_);
        else
            return back_ - front_;
    }
};

}}
