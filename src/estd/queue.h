#pragma once

#include "array.h"
#include "vector.h"
#include "functional.h"
#include "internal/deduce_fixed_size.h"
#include "internal/priority_queue.h"
#include "deque.h"

// deviates from C++ standard queue in that a bool is returned to indicate
// if push/pop succeeds

namespace estd {

// http://en.cppreference.com/w/cpp/container/queue
// we might do just as well using std::queue here, it's mainly the containers which do all the magic
template <class T, class Container>
class queue
{
    Container c;

public:
    queue() = default;
    ESTD_CPP_DEFAULT_RULE_OF_5(queue)

    template<class... Args>
    constexpr explicit queue(in_place_t, Args&&... args) :
        c(in_place_t{}, std::forward<Args>(args)...)
    {}

    ESTD_CPP_STD_VALUE_TYPE(typename Container::value_type)

    typedef typename Container::size_type size_type;
    typedef Container container_type;

    bool empty() const { return c.empty(); }

    size_type size() const { return c.size(); }

    bool pop() { return c.pop_front(); }

    bool push(const_reference value)
    {
        return c.push_back(value);
    }

    reference front() { return c.front(); }

    const_reference front() const { return c.front(); }

    reference back() { return c.back(); }

    const_reference back() const { return c.back(); }

    // emplaces at the back, as per spec
    template <class ...Args>
    reference emplace(Args&&... args)
    {
#if FEATURE_ESTD_GH144
        return *c.emplace_back(std::forward<Args>(args)...);
#else
        return c.emplace_back(std::forward<Args>(args)...);
#endif
    }

    bool push(value_type&& value)
    {
        return c.push_back(std::move(value));
    }
};

namespace layer1 {

template <class T, size_t size>
using queue = estd::queue<T, deque<T, size> >;

}

}
