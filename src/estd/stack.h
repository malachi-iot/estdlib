/**
 * @file
 */
#pragma once

// TODO: should be deque, except eventually plan to decouple layer1 stack completely
#include "queue.h"
#include "vector.h"

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
// layer1::vector actually has all the proper characteristics of a low level stack, but can't
// use it yet because of my alteration to 'back()' behavior in that mine returns accessor which
// doesn't smoothly convert to 'reference'
// However, it *could* if I clean up accessor code a bit and have non-locking accessors expose
// a cast to reference instead of value.  Alternatively could manage returning accessors in some
// cases but not others, but I'd rather the extra complexity of accessors everywhere just to
// maintain consistency
template <class T, size_t N>
class stack : public estd::stack<T, estd::layer1::deque<T, N> >
//class stack : public estd::stack<T, estd::layer1::vector<T, N> >
{
    typedef estd::stack<T, estd::layer1::deque<T, N> > base_type;
    //typedef estd::stack<T, estd::layer1::vector<T, N> > base_type;

public:
    typedef typename base_type::container_type::iterator iterator;
    typedef typename base_type::container_type::const_iterator const_iterator;

    // so that we can iterate from 'bottom' of stack all the way to top
    // spec does not have this functionality
    iterator begin() { return base_type::c.begin(); }
    iterator end() { return base_type::c.end(); }
};

}


namespace layer3 {

// will eventually want layer3 stack flavor also for convenience of passing around
template <class T, typename TSize = size_t>
class stack_experimental {};

}

}
