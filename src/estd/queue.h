#pragma once

#include "../platform.h"
#include "array.h"

// deviates from C++ standard queue in that a bool is returned to indicate
// if push/pop succeeds

namespace estd {

namespace layer1 {

// NOTE: Since layer1 is always a fixed size, this is hard wired as a
// *circular* queue.  Otherwise we'd have a queue which goes to the
// end and basically always fails.  If that's really your desired behavior
// you can recreate that here by checking size() against max_size()
template <class T, size_t size, typename size_t = std::size_t>
class deque
{
    array<T, size> array;
    // TODO: eventually replace these with array iterators
    size_t head;
    size_t tail;

public:
    deque() : head(0), tail(0) {}

    typedef T value_type;
    typedef const value_type& const_reference;
    typedef size_t size_type;

    size_type max_size() const { return array.size(); }

    bool push_back(const T& value)
    {

    }


#ifdef FEATURE_ESTDLIB_MOVESEMANTIC
    bool push_back(const T&& value)
    {

    }
#endif
};




}

// http://en.cppreference.com/w/cpp/container/queue
template <class T, class Container>
class queue
{
    Container container;

public:
    typedef typename Container::value_type value_type;
    typedef typename Container::const_reference const_reference;

    bool push(const_reference value)
    {
        return container.push(value);
    }
};


}
