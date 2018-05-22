//
// Created by malachi on 5/21/18.
//

#pragma once

#include "estd/vector.h"
#include "estd/functional.h"

namespace estd {

// We need our own version of priority queue due to our stateful allocators
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
    typedef typename container_type::accessor accessor;

    bool empty() const { return c.empty(); }

    size_type size() const { return c.size(); }

    accessor top() const
    {
        return c.front();
    }

    void push(const value_type& value)
    {
        c.push_back(value);
        std::push_heap(c.begin(), c.end());
    }


    void pop()
    {
        std::pop_heap(c.begin(), c.end());
        c.pop_back();
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
#endif
};


namespace layer1 {

template <class T, size_t len, class Compare = less<T> >
class priority_queue :
    public estd::priority_queue<T, layer1::vector<T, len>, Compare >
{
public:

};

}


}