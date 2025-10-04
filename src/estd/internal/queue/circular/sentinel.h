#pragma once

#include "base.h"

namespace estd { namespace internal {

template <class Policy>
class circular_queue_impl<Policy, enable_if_t<Policy::type == queue_options::sentinel>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;
    using this_type = circular_queue_impl;

protected:
    using base_type::back_;
    using base_type::front_;
    using base_type::array_;

    constexpr circular_queue_impl() = default;

    template <class ...Args>
    constexpr explicit circular_queue_impl(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

    static bool constexpr clear_size() { return{}; }

public:
    using typename base_type::size_type;

    constexpr bool empty() const
    {
        return front_ == back_;
    }

    size_type size() const
    {
        if(front_ > back_)
            return array_.size() - (front_ - back_);
        else
            return back_ - front_;
    }
};



}}
