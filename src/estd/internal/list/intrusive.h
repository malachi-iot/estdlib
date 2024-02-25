#pragma once

#include "intrusive_forward.h"

namespace estd { namespace internal { namespace list {

// double linked, without tail
template <class T, class Traits = intrusive_traits<T> >
class intrusive : public head_base<T, Traits>
{
    using base_type = head_base<T, Traits>;

public:
    ESTD_CPP_STD_VALUE_TYPE(T)

protected:
    using node_type = pointer;

public:
    EXPLICIT ESTD_CPP_CONSTEXPR_RET intrusive(pointer head = NULLPTR) :
        base_type(head)
    {}
};

template <class T, class Traits = intrusive_traits<T> >
class circular_intrusive : public intrusive<T, Traits>
{

};

}}}