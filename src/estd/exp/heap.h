/**
 * @file
 */
#pragma once

#include "../algorithm.h"
#include "../iterator.h"
#include "../functional.h"

namespace estd { namespace experimental {


/// k-ary heap
/// \tparam RandomIt
/// \tparam Compare
/// reference:
/// https://www.geeksforgeeks.org/k-ary-heap/
template< class RandomIt, class Compare >
struct internal_heap
{
    typedef RandomIt iterator_type;
    typedef estd::iterator_traits<iterator_type> iterator_traits;
    typedef typename iterator_traits::reference reference;

    // remember, in typical std:: fashion 'last' is node *just after*
    // last node
    iterator_type first, last;
    // paradigm is comp(a, b) = true means that a should be moved
    // towards root node.  If one wants a minheap, then make
    // comp(a, b) = a < b
    Compare comp;
    const int k;

    internal_heap(RandomIt first, RandomIt last, const int k = 2, Compare comp = Compare()) :
        first(first),
        last(last),
        comp(comp),
        k(k)
    {}

    // NOTE!! this swap dereferences the expected iterators, so does
    // not swap iterators themselves so much as what they are pointing to
    static void swap(iterator_type a, iterator_type b)
    {
        estd::swap(*a, *b);
    }

    size_t size() const { return last - first; }

    // starting from last element, bubble up
    // returns true when no swapping was needed
    bool restore_up()
    {
        //typedef typename iterator_traits::value_type value_type;

        const int last_idx = size();
        //iterator_type current = last - sizeof(value_type);
        int current_idx = last_idx - 1;
        iterator_type current = first + current_idx;

        while(current != first)
        {
            int parent_idx = (current_idx - 1) / k;
            iterator_type parent = first + parent_idx;

            // if current actually should bubble up
            if(comp(*current, *parent))
            {
                swap(current, parent);
                current = parent;
                current_idx = parent_idx;
            }
            else
                // if no bubble up needed, then we're totally done
                break;
        }

        // true if we never moved current_idx, meaning no swaps happened
        return current_idx == last_idx - 1;
    }

    // push down
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
    constexpr
#endif
    void restore_down(iterator_type current)
    {
        for(;;)
        {
            int current_idx = current - first;
            int first_child_idx = (current_idx * k) + 1;
            iterator_type first_child = first + first_child_idx;

            // past the end of the leaf
            if(first_child >= last) break;

            // theoretical iterator one past last child - it may actually be even
            // further past the end.
            iterator_type last_child = estd::min(first_child + k, last);

            iterator_type chosen_child = estd::min_element(first_child, last_child, comp);

            // now, if the best candidate child also should be bumped up
            if(comp(*chosen_child, *current))
            {
                // then swap and bump it up
                swap(chosen_child, current);
                //current = next_node; // move down the heap
                current = chosen_child;
            }
            else
                // if current is already best option, then we're done
                break;
        }
    }

    // starting from first element, push down
    void restore_down()
    {
        restore_down(first);
    }

    const reference front() const { return *first; }

    void pop()
    {
        swap(first, --last);
        restore_down();
    }

    // NOTE: be careful, last++ may not produce correct results
    void push(const typename iterator_traits::value_type& v)
    {
        *last++ = v;
        restore_up();
    }


    // use this when iterator has substantially changed (like
    // when a new linked list node has been allocated for the
    // new last-1 position)
    void push(iterator_type last)
    {
        this->last = last;
        restore_up();
    }


#ifdef FEATURE_CPP_CONSTEXPR_METHOD
    constexpr
#endif
    void make()
    {
        int last_nonleaf_idx = (size() - 1) / k;
        // FIX: this is not sufficient, because restore_up doesn't evaluate all children
        //while(!restore_up());
        for(iterator_type i = first + last_nonleaf_idx;
            ;)
        {
            restore_down(i);

            if(i-- == first) break;
        }
    }
};

template< class RandomIt, class Compare >
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
void make_heap( RandomIt first, RandomIt last, Compare comp, const int k = 2 )
{
    internal_heap<RandomIt, Compare> heap(first, last, k, comp);

    heap.make();
}

}}
