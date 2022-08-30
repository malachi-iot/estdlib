//
// Created by malachi on 5/21/18.
//

#pragma once

#include "platform.h"

#include "../exp/heap.h"

// relies on std::push_heap
#ifdef FEATURE_STD_ALGORITHM

#define FEATURE_ESTD_PRIORITY_QUEUE 1

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
class priority_queue : protected estd::internal::struct_evaporator<Compare>
{
    typedef estd::internal::struct_evaporator<Compare> compare_provider_type;

protected:
    Container c;

public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::reference reference;
    typedef typename Container::size_type size_type;
    typedef typename container_type::accessor accessor;

    ESTD_CPP_DEFAULT_CTOR(priority_queue)

    priority_queue(const Compare& compare) :
        compare_provider_type(compare)
    {}

    bool empty() const { return c.empty(); }

    size_type size() const { return c.size(); }

    // DEBT: Consider returning a const reference here instead, since returned
    // value is expected to be used and discarded rather quickly.  So theoretically
    // moved memory under a weak lock might be acceptable
    accessor top() const
    {
        return c.front();
    }

    void push(const value_type& value)
    {
        c.push_back(value);
        std::push_heap(c.begin(), c.end(), compare_provider_type::value());
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    void push(value_type&& value)
    {
        c.push_back(std::move(value));
        std::push_heap(c.begin(), c.end(), compare_provider_type::value());
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
        std::pop_heap(c.begin(), c.end(), compare_provider_type::value());
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

    /// Experimental call which calls `f` with accessor for the emplaced
    /// item before it gets heapified
    /// @tparam F
    /// @tparam TArgs
    /// @param f
    /// @param args
    /// @return
    template <class F, class ...TArgs>
    accessor emplace_with_notify(F f, TArgs&&...args)
    {
        auto accessor = c.emplace_back(std::forward<TArgs>(args)...);
        f(accessor.lock());
        accessor.unlock();
        std::push_heap(c.begin(), c.end(), Compare());
        return c.back();
    }
#endif

    // EXPERIMENTAL
    container_type& container() { return c; }
    const container_type& container() const { return c; }

    // EXPERIMENTAL
    // https://www.geeksforgeeks.org/insertion-and-deletion-in-heaps/
    void erase(reference v)
    {
        v = c.back();
        c.pop_back();
        // DEBT: Slow!  make_heap does multiple heapify and we only need one
        // see https://stackoverflow.com/questions/32213377/heapify-in-c-stl
        // DEBT: Also, we want to do this from 'v' not c.begin().  However,
        // our accessor-rather-than-pointer makes that a little tricky
        std::make_heap(c.begin(), c.end(), Compare());
    }

#ifdef __cpp_rvalue_references
    // EXPERIMENTAL
    template <class F>
    bool erase2(F&& f)
    {
        auto i = estd::find_if(c.begin(), c.end(), f);

        if(i == c.end()) return false;

        erase(*i);
        return true;
    }
#endif
};


namespace layer1 {

template <class T, size_t len, class Compare = less<T> >
class priority_queue :
    public estd::priority_queue<T, layer1::vector<T, len>, Compare >
{
    typedef estd::priority_queue<T, layer1::vector<T, len>, Compare > base_type;

public:
    ESTD_CPP_DEFAULT_CTOR(priority_queue)

    priority_queue(const Compare& compare) : base_type(compare)
    {

    }
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
