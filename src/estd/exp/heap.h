/**
 * @file
 */
#pragma once

#include "../iterator.h"

namespace estd { namespace experimental {


/// k-ary heap
/// \tparam RandomIt
/// \tparam Compare
template< class RandomIt, class Compare >
struct internal_heap
{
    typedef RandomIt iterator_type;
    typedef estd::iterator_traits<iterator_type> iterator_traits;

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
    void restore_up()
    {
        RandomIt current = last;
        int current_idx = last - first;

        while(current != first)
        {
            RandomIt parent = first + ((--current_idx) / k);

            // if current actually should bubble up
            if(comp(current, parent))
            {
                swap(current, parent);
                current = parent;
            }
            else
                // if no bubble up needed, then we're totally done
                break;
        }
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
                if(comp(child, chosen_child))
                    chosen_child = child;
            }

            if(comp(chosen_child, current))
            {
                swap(chosen_child, current);
                current = last_child; // move down the heap
            }
            else
                break;
        }
    }
};

template< class RandomIt, class Compare >
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
void make_heap( RandomIt first, RandomIt last, Compare comp, const int k = 2 )
{
    internal_heap<RandomIt, Compare> heap(first, last, comp, k);
}

}}