#pragma once

#include "fwd.h"

// In response to https://github.com/malachi-iot/estdlib/issues/11

namespace estd { namespace internal { namespace list {

template <class T, class Traits = intrusive_traits<T> >
struct intrusive_iterator
{
    using iterator = intrusive_iterator;
    using nonconst_iterator = intrusive_iterator<typename remove_const<T>::type >;
    using const_iterator = intrusive_iterator<typename add_const<T>::type >;

    ESTD_CPP_STD_VALUE_TYPE(T)

    pointer current_;

public:
    // Unclear whether this is recommended, but sure is useful - and doesn't violate any
    // specific rules.  Conversion from non const -> const or non const
    ESTD_CPP_CONSTEXPR_RET intrusive_iterator(const nonconst_iterator& copy_from) : // NOLINT
        current_{copy_from.current_}
    {}

    ESTD_CPP_CONSTEXPR_RET intrusive_iterator(pointer start) :
        current_(start)
    {}

    ESTD_CPP_CONSTEXPR_RET intrusive_iterator(null_iterator) :
        current_(nullptr)
    {}

    //ESTD_CPP_CONSTEXPR_RET intrusive_iterator(const iterator& copy_from) = default;

    iterator& operator++()
    {
        current_ = Traits::next(current_);
        return *this;
    }

    iterator operator++(int)
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

    iterator operator--(int)
    {
        iterator temp(current_);
        --*this;
        return temp;
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
    const_reference operator*() const { return Traits::data(current_); }
};


}}}