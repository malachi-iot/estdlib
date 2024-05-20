//
// Created by malachi on 5/21/18.
//

#pragma once

#include "platform.h"

#include "../exp/heap.h"
#include "feature/queue.h"
#include "algorithm/heap.h"

// DEBT: Obsolete, we now always have priority queue since we have our own
// heap implementation
#define FEATURE_ESTD_PRIORITY_QUEUE 1

#include "../vector.h"
#include "../functional.h"

#if !FEATURE_ESTD_ALGORITHM_HEAP
#include <algorithm>
#endif

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

    void push_heap()
    {
#if FEATURE_ESTD_ALGORITHM_HEAP
        estd::experimental::push_heap(c.begin(), c.end(), compare());
#else
        std::push_heap(c.begin(), c.end(), compare());
#endif
    }

    void pop_heap()
    {
#if FEATURE_ESTD_ALGORITHM_HEAP
        estd::experimental::pop_heap(c.begin(), c.end(), compare());
#else
        std::pop_heap(c.begin(), c.end(), compare());
#endif
    }

public:
    // 'evaporated' type which means either Compare itself if empty, or
    // a reference to Compare if not empty
    typedef typename compare_provider_type::evaporated_type compare_type;

    // EXPERIMENTAL making this public, embedded environments can benefit from the few bytes
    // saved by not passing around pointers for more advanced compare behavior
    inline compare_type compare() { return compare_provider_type::value(); }

    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::reference reference;
    typedef typename Container::size_type size_type;
    typedef typename container_type::accessor accessor;

    ESTD_CPP_DEFAULT_CTOR(priority_queue)

    EXPLICIT priority_queue(const Compare& compare) :
        compare_provider_type(compare)
    {}

    ESTD_CPP_CONSTEXPR_RET bool empty() const { return c.empty(); }

    ESTD_CPP_CONSTEXPR_RET size_type size() const { return c.size(); }

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
        push_heap();
    }

#ifdef __cpp_rvalue_references
    void push(value_type&& value)
    {
        c.push_back(std::move(value));
        push_heap();
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
        pop_heap();
        c.pop_back();
    }


#if __cpp_rvalue_references && __cpp_variadic_templates
    template <class ...TArgs>
    accessor emplace(TArgs&&...args)
    {
        c.emplace_back(std::forward<TArgs>(args)...);
        push_heap();
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
        push_heap();
        return c.back();
    }
#endif

    // EXPERIMENTAL
    container_type& container() { return c; }
    const container_type& container() const { return c; }

    // https://www.geeksforgeeks.org/insertion-and-deletion-in-heaps/
    // DEBT: Not optimized, but passes tests
    void erase(reference v)
    {
        // Wipe out what v was pointing to with content of last
        // node
        v = c.back();

        // Now, erase last (duplicated) node and rebalance heap
        c.pop_back();
        // DEBT: Slow!  make_heap does multiple heapify and we only need one
        // see https://stackoverflow.com/questions/32213377/heapify-in-c-stl
        // DEBT: Also, we want to do this from 'v' not c.begin().  However,
        // our accessor-rather-than-pointer makes that a little tricky
#if FEATURE_ESTD_ALGORITHM_HEAP
        estd::experimental::make_heap(c.begin(), c.end(), compare());
#else
        std::make_heap(c.begin(), c.end(), compare());
#endif
    }

#ifdef __cpp_rvalue_references
    template <class F>
    bool erase_if(F&& f)
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
    public estd::priority_queue<T, layer1::vector<T, len>, Compare>
{
    typedef estd::priority_queue<T, layer1::vector<T, len>, Compare> base_type;

public:
    ESTD_CPP_DEFAULT_CTOR(priority_queue)

    EXPLICIT priority_queue(const Compare& compare) : base_type(compare)
    {

    }
};

}


namespace layer2 {

template <class T, size_t len, class Compare = less<T> >
class priority_queue :
        public estd::priority_queue<T, layer2::vector<T, len>, Compare>
{
public:

};

}

}
