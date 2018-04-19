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

    array_t m_array;

    typedef typename array_t::iterator array_iterator;

    // front aka head aka 'leftmost' part of array,
    //   where items are traditionally retrieved
    // back aka tail aka 'rightmost' part of array,
    //   where items are traditionally added
    array_iterator m_front;
    array_iterator m_back;

    // because I don't want to waste a slot
    bool m_empty;

    void evaluate_rollover(array_iterator* i)
    {
        if(*i == m_array.end())
            *i = m_array.begin();
    }

    // FIX: Something is wrong, m_front
    // never registers as > m_back.  Might just
    // be permissive test?
    size_t _size() const
    {
        if(m_front > m_back)
            return N - (m_front - m_back);
        else
            return m_back - m_front;
    }

public:
    deque() :
        m_front(m_array.begin()),
        m_back(m_array.begin()),
        m_empty(true) {}

    typedef T value_type;
    typedef const value_type& const_reference;
    typedef size_t size_type;

    bool empty() const { return m_empty; }

    size_type size() const
    {
        if(empty()) return 0;

        size_type s = _size();

        if(s == 0) s = max_size();

        return s;
    }

    size_type max_size() const { return m_array.size(); }

    bool push_back(const T& value)
    {
        *m_back++ = value;
        m_empty = false;

        evaluate_rollover(&m_back);

        return true;
    }


    bool push_front(const T& value)
    {
        *m_front++ = value;

        evaluate_rollover(&m_front);
        m_empty = false;

        return true;
    }


    bool pop_front()
    {
        m_front++;

        evaluate_rollover(&m_front);

        if(m_front == m_back) m_empty = true;

        return true;
    }

    const_reference front() const
    {
        return *m_front;
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    bool push_back(const T&& value)
    {
        *m_back++ = value;

        evaluate_rollover(&m_back);
        m_empty = false;

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

#ifdef FEATURE_CPP_MOVESEMANTIC
    bool push(value_type&& value)
    {
        return c.push_back(value);
    }
#endif
};


}
