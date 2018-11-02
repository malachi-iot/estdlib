/**
 * @file
 */
#pragma once

#include "../iterator.h"

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

    iterator_type first, last;
    // paradigm is comp(a, b) = true means that a should be moved
    // towards root node.  If one wants a minheap, then make
    // comp(a, b) = a < b
    Compare comp;
    const int k;

    internal_heap(RandomIt first, RandomIt last, Compare comp, const int k) :
        first(first),
        last(last),
        comp(comp),
        k(k)
    {}

    // NOTE!! this swap dereferences the expected iterators, so does
    // not swap iterators themselves so much as what they are pointing to
    static void swap(iterator_type a, iterator_type b)
    {
        typedef typename iterator_traits::value_type value_type;
        //typedef typename iterator_traits::reference reference;

        value_type temp = *a;

        *a = *b;
        *b = temp;
    }

    // starting from last element, bubble up
    // FIX: I think it's supposed to bubble up from a specific element
    // returns true when no swapping was needed
    bool restore_up()
    {
        typedef typename iterator_traits::value_type value_type;

        //iterator_type current = last - sizeof(value_type);
        // FIX: this value_type participation is not going to work with all iterator types
        int last_idx = (last - first) / sizeof(value_type);
        int current_idx = last_idx - 1;
        iterator_type current = first + current_idx;

        while(current != first)
        {
            int parent_idx = (current_idx - 1) / k;
            RandomIt parent = first + parent_idx;

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

    // starting from first element, push down
    void restore_down()
    {
        iterator_type current = first;

        for(;;)
        {
            iterator_type last_child = current + k;
            iterator_type chosen_child = current + 1;

            for(iterator_type child = chosen_child + 1; child < last_child; child++)
            {
                // find best candidate child
                // (minheap this would be child with smallest value)
                if(comp(*child, *chosen_child))
                    chosen_child = child;
            }

            if(comp(*chosen_child, *current))
            {
                swap(chosen_child, current);
                current = last_child; // move down the heap
            }
            else
                break;
        }
    }

    const reference front() const { return *first; }

    void pop()
    {
        // UNTESTED
        swap(first, last);
        last--;
        restore_down();
    }
};

template< class RandomIt, class Compare >
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
void make_heap( RandomIt first, RandomIt last, Compare comp, const int k = 2 )
{
    internal_heap<RandomIt, Compare> heap(first, last, comp, k);

    while(!heap.restore_up());
}

}}