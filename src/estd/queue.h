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
template <class T, size_t N, typename size_t = std::size_t>
class deque
{
    typedef array<T, N> array_t;

    array_t array;
    typename array_t::iterator m_front;
    typename array_t::iterator m_back;

public:
    deque() :
        m_front(array.begin()),
        m_back(array.begin()) {}

    typedef T value_type;
    typedef const value_type& const_reference;
    typedef size_t size_type;

    size_type max_size() const { return array.size(); }

    size_type size() const
    {
        return m_front - m_back;
    }

    bool push_back(const T& value)
    {
        *m_back++ = value;
        return true;
    }


    bool push_front(const T& value)
    {
        *m_front++ = value;
        return true;
    }


    bool pop_front()
    {
        m_front++;
        return true;
    }

    const_reference front() const
    {
        return *m_front;
    }

#ifdef FEATURE_ESTDLIB_MOVESEMANTIC
    bool push_back(const T&& value)
    {
        *m_back++ = value;
        return true;
    }
#endif
};




}

// http://en.cppreference.com/w/cpp/container/queue
template <class T, class Container>
class queue
{
    Container c;

public:
    typedef typename Container::value_type value_type;
    typedef typename Container::const_reference const_reference;
    typedef typename Container::size_type size_type;

    size_type size() const { return c.size(); }

    bool pop() { return c.pop_front(); }

    bool push(const_reference value)
    {
        return c.push_back(value);
    }

    const_reference front() const { return c.front(); }

#ifdef FEATURE_ESTDLIB_MOVESEMANTIC
    bool push(value_type&& value)
    {
        return c.push_back(value);
    }
#endif
};


}
