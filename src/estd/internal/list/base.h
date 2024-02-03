#pragma once

#include "iterator.h"

// In response to https://github.com/malachi-iot/estdlib/issues/11

namespace estd { namespace internal { namespace list {

template <class T, class Traits>
class head_base
{
protected:
    ESTD_CPP_STD_VALUE_TYPE(T)

    using node_type = pointer;

    node_type head_;

public:
    // Intrusive lists have a very different character for initialization
    // than their regular std counterparts
    EXPLICIT ESTD_CPP_CONSTEXPR_RET head_base(pointer head = NULLPTR) :
        head_(head)
    {}

    using iterator = intrusive_iterator<T, Traits>;
    // DEBT: Still needs love, a pointer isn't gonna quite const up right here I think
    using const_iterator = intrusive_iterator<const T, Traits>;

    ESTD_CPP_CONSTEXPR_RET bool empty() const
    {
        return head_ == nullptr;
    }

    iterator begin()
    {
        return iterator(head_);
    }

    const_iterator begin() const
    {
        return const_iterator(head_);
    }

    const_iterator cbegin() const
    {
        return const_iterator(head_);
    }

    // range operator wants it this way.  Could do a fancy conversion from
    // null_iterator back to regular iterator too...
    ESTD_CPP_CONSTEXPR_RET iterator end() const
    {
        return iterator(nullptr);
    }

    ESTD_CPP_CONSTEXPR_RET null_iterator cend() const { return null_iterator(); }

    reference front()
    {
        return *head_;
    }
};


}}}