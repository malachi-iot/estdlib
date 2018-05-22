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

// std::priority_queue
// was freaking out and not able to allocate as expected.  On further insepction,
// std::priorty_queue does maintain a discrete 'Container c', so not sure what the
// problem is
template <
        class T,
        class Container = vector<T>,
        class Compare = less<typename Container::value_type>,
        class THelper = internal::priority_queue_helper<Container, Compare> >
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

#ifdef FEATURE_CPP_MOVESEMANTIC
    void push(value_type&& value)
    {
        c.push_back(std::forward<value_type>(value));
        std::push_heap(c.begin(), c.end(), Compare());
    }
#endif


    void pop()
    {
        std::pop_heap(c.begin(), c.end(), Compare());
        c.pop_back();
    }


#if defined(FEATURE_CPP_MOVESEMANTIC) && defined(FEATURE_CPP_VARIADIC)
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


namespace layer2 {

template <class T, size_t len, class Compare = less<T> >
class priority_queue :
        public estd::priority_queue<T, layer2::vector<T, len>, Compare >
{
public:

};

}

}