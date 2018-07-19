//
// Created by malachi on 5/21/18.
//

#pragma once

#include "platform.h"

// relies on std::push_heap
#ifdef FEATURE_STD_ALGORITHM

#include "../vector.h"
#include "../functional.h"

#include <algorithm>

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
#ifdef FEATURE_STD_MEMORY
        class Container = vector<T>,
#else
        class Container,
#endif
        class Compare = less<typename Container::value_type>,
        class THelper = internal::priority_queue_helper<Container, Compare> >
class priority_queue
{
protected:
    Container c;

    Compare get_compare() { return Compare(); }

public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename container_type::accessor accessor;
    typedef Compare value_compare;

    bool empty() const { return c.empty(); }

    size_type size() const { return c.size(); }

    accessor top() const
    {
        return c.front();
    }

    void push(const value_type& value)
    {
        c.push_back(value);
        std::push_heap(c.begin(), c.end(), get_compare());
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    void push(value_type&& value)
    {
        c.push_back(std::move(value));
        std::push_heap(c.begin(), c.end(), get_compare());
    }

    priority_queue& operator =(priority_queue&& move_from)
    {
        c = std::move(move_from.c);
        return *this;
    }
#endif

    priority_queue& operator =(const priority_queue& copy_from)
    {
        c = copy_from.c;
        return *this;
    }

    void pop()
    {
        std::pop_heap(c.begin(), c.end(), get_compare());
        c.pop_back();
    }


#if defined(FEATURE_CPP_MOVESEMANTIC) && defined(FEATURE_CPP_VARIADIC)
    template <class ...TArgs>
    accessor emplace(TArgs&&...args)
    {
        c.emplace_back(std::forward<TArgs>(args)...);
        std::push_heap(c.begin(), c.end(), Compare());
        return c.back();
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

#endif
