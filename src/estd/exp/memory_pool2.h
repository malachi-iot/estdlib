#pragma once

#include <cstddef>

#include "../internal/deduce_fixed_size.h"
#include "../memory.h"  // for platform + raw_instance_provider
#include "../array.h"
#include "../algorithm.h"

namespace estd { namespace experimental {

// regular memory_pool.h seems to work well, but was done very long ago and I can't remember
// enough about its underlying architecture to proceed comfortably.
namespace v2 {

enum class pool_types
{
    linked_list,
    handle
};

template <class T, std::size_t sz, pool_types type>
class memory_pool;

template <std::size_t N>
struct memory_pool_base
{
    typedef typename internal::deduce_fixed_size_t<N>::size_type size_type;
};


template <class T, std::size_t N>
class memory_pool<T, N, pool_types::linked_list>
{
private:
    union item
    {
        item* next;
        // NOTE: Usage of estd::array may implicitly bring along
        // raw_instance_provider behavior.
        estd::experimental::raw_instance_provider<T> value;
    };

    estd::array<item, N> items;
    item* head;

public:
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    memory_pool()
    {
        item* i = items.begin();

        head = i;

        while(i != items.end())
        {
            // due to
            // https://stackoverflow.com/questions/26351162/will-right-hand-side-of-an-expression-always-evaluated-first
            // can't quite do a one-liner
            //i->next = ++i;

            item* temp = i;
            temp->next = ++i;
        }
    }

    // TODO: Do constructor flavor
    value_type* allocate()
    {
        if(head == items.end()) return NULLPTR;

        item* i = head;

        head = head->next;

        return &i->value.value();
    }

    void deallocate(value_type* value)
    {
        item* i = reinterpret_cast<item*>(value);

        i->next = head;
        head = i;
    }

    std::size_t count_free()
    {
        item* i = head;
        std::size_t count = 0;

        // TODO: Make a estd::accumulate
        for(; i != items.end(); ++i, ++count);

        return count;
    }

    // somewhat internal call used mainly down the line for aggregated pools,
    // does not indicate a valid pointer, only that the pointer does reside in
    // the pool range
    bool within_range(value_type* value)
    {
        return value >= items.begin() && value < items.end();
    }
};

/*
 *  perhaps not needed if above can conform to allocator signature/concept
 *  https://en.cppreference.com/w/cpp/memory/allocator
template <class T, std::size_t N, pool_types type>
class pool_allocator
{
private:
    memory_pool<T, N, type> pool;

public:

}; */

}

}}