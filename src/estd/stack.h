/**
 * @file
 */
#pragma once

// TODO: should be deque, except eventually plan to decouple layer1 stack completely
#include "queue.h"

namespace estd {

template <class T, class TContainer>
class stack
{
protected:
    TContainer c;

public:
    typedef TContainer container_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::reference reference;
    typedef typename container_type::const_reference const_reference;

    void push(const value_type& value) { c.push_back(value); }
    void pop() { c.pop_back(); }

    reference top() { return c.back(); }
    const_reference top() const { return c.back(); }

    size_type size() const { return c.size(); }
    bool empty() const { return c.empty(); }

    stack() {}

    explicit stack( const TContainer& cont ) : c(cont) {}
};

namespace layer1 {

// TODO: eventually make this standalone and probably actually have layer1::deque based on THIS
template <class T, size_t N>
class stack : public estd::stack<T, estd::layer1::deque<T, N> >
{
    typedef estd::stack<T, estd::layer1::deque<T, N> > base_type;

public:
    typedef typename base_type::container_type::iterator iterator;
    typedef typename base_type::container_type::const_iterator const_iterator;

    // so that we can iterate from 'bottom' of stack all the way to top
    // spec does not have this functionality
    iterator begin() { return base_type::c.begin(); }
    iterator end() { return base_type::c.end(); }
};

}

}
