#pragma once

#include "../platform.h"
#include "array.h"
#include "vector.h"
#include "functional.h"

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

    // TODO: Make special iterator for dequeue which does the rollover/rollunder
    // and checks that you only move back and forth within one 'session' of
    // elements
    typedef typename array_t::iterator array_iterator;

    // front aka head aka 'leftmost' part of array,
    //   where items are traditionally retrieved
    // back aka tail aka 'rightmost' part of array,
    //   where items are traditionally added
    array_iterator m_front;
    array_iterator m_back;

    // because I don't want to waste a slot
    bool m_empty;

    // called when i is incremented, evaluates if i reaches rollover point
    // and if so points it back at the beginning
    void evaluate_rollover(array_iterator* i)
    {
        if(*i == m_array.end())
            *i = m_array.begin();
    }

    // called when i is decremented, opposite of rollover check
    // returns true when a rollover is detected.  Returns whether
    // we rolled over to help with decrement (again, artifact
    // of no cbegin)
    bool evaluate_rollunder(array_iterator* i)
    {
        if(*i == m_array.begin())
        {
            *i = m_array.end();
            return true;
        }
        else
            return false;
    }

    // have to do these increment/decrements out here because array iterator itself
    // wouldn't handle rollovers/rollunders
    void decrement(array_iterator* i)
    {
        // doing i-- after because we don't have a serviceable cbegin just yet
        if(!evaluate_rollunder(i))
            (*i)--;
    }

    // handle rollover and empty-flag set for iterator
    void increment(array_iterator* i, bool empty = false)
    {
        (*i)++;
        evaluate_rollover(i);
        m_empty = empty;
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

    typedef array_iterator iterator; // I think we should use this one all the way around, not sure why I have an explicit 'array_iterator'
    typedef T value_type;
    typedef value_type& reference;
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

    reference front()
    {
        return *m_front;
    }

    const_reference front() const
    {
        return *m_front;
    }

    reference back()
    {
        iterator i = m_back;

        decrement(&i);

        return *i;
    }

    const_reference back() const
    {
        iterator i = m_back;

        decrement(&i);

        return *i;
    }

#ifdef FEATURE_CPP_VARIADIC
    // Compiles but untested
    template <class ...TArgs>
    reference emplace_front(TArgs...args)
    {
        value_type& retval = *m_front;

        new (&retval) value_type(args...);

        increment(&m_front);

        return retval;
    }

    // Compiles but untested
    template <class ...TArgs>
    reference emplace_back(TArgs...args)
    {
        value_type& retval = *m_back;

        new (&retval) value_type(args...);

        increment(&m_back);

        return retval;
    }
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
    bool push_back(value_type&& value)
    {
        value_type& back = *m_back++;

        new (&back) value_type(std::forward<value_type>(value));

        evaluate_rollover(&m_back);
        m_empty = false;

        return true;
    }
#endif
};




}

// http://en.cppreference.com/w/cpp/container/queue
// we might do just as well using std::queue here, it's mainly the containers which do all the magic
template <class T, class Container>
class queue
{
    Container c;

public:
    typedef typename Container::value_type value_type;
    typedef typename Container::const_reference const_reference;
    typedef typename Container::reference reference;
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

#ifdef FEATURE_CPP_VARIADIC
    // emplaces at the back, as per spec
    template <class ...TArgs>
    reference emplace(TArgs...args)
    {
        return c.emplace_back(args...);
    }
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
    bool push(value_type&& value)
    {
        return c.push_back(std::forward<value_type>(value));
    }
#endif
};

namespace layer1 {

template <class T, size_t size>
class queue : public estd::queue<T, layer1::deque<T, size> >
{
};

};

template <
        class T,
        class Container = vector<T>,
        class Compare = less<typename Container::value_type> >
class priority_queue
{
protected:
    Container c;

public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;

    bool empty() const { return c.empty(); }

    size_type size() const { return c.size(); }
};


};
