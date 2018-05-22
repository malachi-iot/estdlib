//
// Created by malachi on 5/21/18.
//

#pragma once

#include "estd/vector.h"
#include "estd/functional.h"

namespace estd {

namespace internal {

template <class Container, class Compare>
class priority_queue_helper;

}

// We need our own version of priority queue due to our stateful allocators
// NOTE: maybe not, but sure seemed that way - seemed like std::priority_queue
// was freaking out and not able to allocate as expected
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
        std::push_heap(c.begin(), c.end(), Compare());
    }


    void pop()
    {
        std::pop_heap(c.begin(), c.end(), Compare());
        c.pop_back();
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
    template <class ...TArgs>
    accessor emplace(TArgs&&...args)
    {
        c.emplace_back(std::forward<TArgs>(args)...);
        std::push_heap(c.begin(), c.end(), Compare());
    }
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