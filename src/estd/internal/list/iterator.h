#pragma once

#include "fwd.h"

// In response to https://github.com/malachi-iot/estdlib/issues/11

namespace estd { namespace internal { namespace list {

template <class T, class Traits = intrusive_traits<T> >
struct intrusive_iterator
{
    using iterator = intrusive_iterator;

    ESTD_CPP_STD_VALUE_TYPE(T)

    pointer current_;

public:
    ESTD_CPP_CONSTEXPR_RET intrusive_iterator(pointer start) :
        current_(start)
    {}

    ESTD_CPP_CONSTEXPR_RET intrusive_iterator(null_iterator) :
        current_(nullptr)
    {}

    iterator& operator++()
    {
        current_ = Traits::next(current_);
        return *this;
    }

    iterator operator++(int) const
    {
        iterator temp(current_);
        ++*this;
        return temp;
    }

    iterator& operator--()
    {
        current_ = Traits::prev(current_);
        return *this;
    }

    bool operator==(null_iterator) const
    {
        return current_ == nullptr;
    }

    bool operator!=(null_iterator) const
    {
        return current_ != nullptr;
    }

    bool operator==(const iterator& compare_to) const
    {
        return current_ == compare_to.current_;
    }

    bool operator!=(const iterator& compare_to) const
    {
        return current_ != compare_to.current_;
    }

    reference operator*() { return Traits::data(current_); }
};


}}}