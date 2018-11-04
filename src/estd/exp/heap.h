/**
 * @file
 */
#pragma once

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
        typedef typename iterator_traits::value_type value_type;
        //typedef typename iterator_traits::reference reference;

        value_type temp = *a;

        *a = *b;
        *b = temp;
    }

    int last_idx() const
    {
        int i = last - first;
        return i;
    }

    // starting from last element, bubble up
    // FIX: I think it's supposed to bubble up from a specific element
    // returns true when no swapping was needed
    bool restore_up()
    {
        //typedef typename iterator_traits::value_type value_type;

        const int last_idx = this->last_idx();
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
    void restore_down(iterator_type current)
    {
        // last actual node present
        iterator_type last_node = last - 1;

        // only proceed if there is more down to go
        while(current != last_node)
        {
            int current_idx = current - first;
            int first_child_idx = (current_idx * k) + 1;
            iterator_type first_child = first + first_child_idx;
            // theoretical iterator to last child - it may actually be past the end
            // FIX: adjust last child based on last_node
            iterator_type last_child = first_child + k - 1;
            // start of children.  first child automatically assigned to chosen
            iterator_type chosen_child = first_child;
            iterator_type next_node = last_child + 1;

            // FIX: I think we need to eliminate this.  a child in this position
            // might actually want to be swapped
            if(chosen_child > last_node) break;

            for(iterator_type child = chosen_child;
                child <= last_child;
                child++)
            {
                // find best candidate child
                // (minheap this would be child with smallest value)
                if(comp(*child, *chosen_child))
                    chosen_child = child;

                // if, after evaluating this child, we discover it's the last_node,
                // then we are done
                if(child == last_node)
                {
                    next_node = last_node;
                    break;
                }
            }

            // now, if the best candidate child also should be bumped up
            if(comp(*chosen_child, *current))
            {
                // then swap and bump it up
                swap(chosen_child, current);
                current = next_node; // move down the heap
            }
            else
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

    void make()
    {
        int last_nonleaf_idx = (last_idx() - 1) / k;
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

    while(!heap.restore_up());
}

}}