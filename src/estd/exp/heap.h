/**
 * @file
 */
#pragma once

#include "../algorithm.h"
#include "../internal/iterator_standalone.h"
#include "../functional.h"

namespace estd { namespace experimental {


/// k-ary heap
/// \tparam RandomIt
/// \tparam Compare
/// reference:
/// https://www.geeksforgeeks.org/k-ary-heap/
template< class RandomIt, class Compare, unsigned k = 2 >
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

    internal_heap(RandomIt first, RandomIt last, Compare comp = Compare()) :
        first(first),
        last(last),
        comp(comp)
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
    // aka sift up
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

    // push down aka sift down
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

    // evaluate the 'top' elemant as:
    // 1. a pop
    // 2. an immediate push back of the same 'top' memory spot
    // this is useful if you have in-placed updated the element and wish to re-insert it.
    // use case: reschedule
    // Similar to https://docs.python.org/3/library/heapq.html#heapq.heapreplace
    void posh()
    {
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

    // FIX: Does not do what we want - do not use
    void sift_down(unsigned idx)
    {
        iterator_type i = first + idx;
        iterator_type largest = i;

        iterator_type left_child = first + ((idx * 2) + 1);
        //iterator_type right_child = left_child + 1;
        iterator_type right_child = first + ((idx * 2) + 2);

        if(!comp(*left_child, *largest))
        {
            largest = left_child;
        }
        else if(right_child < last && !comp(*right_child, *largest))
        {
            largest = right_child;
        }
        else
        {
            return;
        }

        swap(i, largest);
        sift_down(idx);
    }

    // FIX: Does not do what we want - do not use
    void sift_down()
    {
       // sift_down(0);


        unsigned first_nonleaf_idx = (size() / k) - 1;
        //unsigned first_nonleaf_idx = (size() - 1) / k;

        for(unsigned i = 0; i < first_nonleaf_idx;++i)
        {
            sift_down(i);
        }
    }


    // from https://www.programiz.com/dsa/heap-data-structure
    // and https://www.geeksforgeeks.org/cpp-program-for-heap-sort/
    void heapify(unsigned idx)
    {
        iterator_type i = first + idx;
        iterator_type largest = i;

        iterator_type left_child = first + ((idx * 2) + 1);
        //iterator_type right_child = left_child + 1;
        iterator_type right_child = first + ((idx * 2) + 2);

        if(comp(*left_child, *largest))
        {
            largest = left_child;
        }
        else if(right_child < last && comp(*right_child, *largest))
        {
            largest = right_child;
        }
        else
        {
            return;
        }

        swap(i, largest);
        heapify(idx);
    }

    // from https://www.programiz.com/dsa/heap-data-structure
    void make2()
    {
        unsigned first_nonleaf_idx = (size() / k) - 1;
        //unsigned first_nonleaf_idx = (size() - 1) / k;

        for(unsigned i = first_nonleaf_idx;;--i)
        {
            heapify(i);

            if(i == 0) break;
        }
    }

    reference pop2()
    {
        pop();
        return *first;
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
void make_heap( RandomIt first, RandomIt last, Compare comp)
{
    internal_heap<RandomIt, Compare> heap(first, last, comp);

    heap.make();
}

// following is from
// https://stackoverflow.com/questions/32672474/how-to-replace-top-element-of-heap-efficiently-withouth-re-establishing-heap-inv

template <typename DifferenceT>
DifferenceT heap_parent(DifferenceT k)
{
    return (k - 1) / 2;
}

template <typename DifferenceT>
DifferenceT heap_left(DifferenceT k)
{
    return 2 * k + 1;
}

template<typename RandomIt,
    typename Compare = estd::less<typename estd::iterator_traits<RandomIt>::value_type> >
void heapreplace(RandomIt first, RandomIt last, Compare comp = Compare())
{
    auto const size = last - first;
    if (size <= 1)
        return;
    typename std::iterator_traits<RandomIt>::difference_type k = 0;
    auto e = std::move(first[k]);
    auto const max_k = heap_parent(size - 1);
    while (k <= max_k) {
        auto max_child = heap_left(k);
        if (max_child < size - 1 && comp(first[max_child], first[max_child + 1]))
            ++max_child; // Go to right sibling.
        if (!comp(e, first[max_child]))
            break;
        first[k] = std::move(first[max_child]);
        k = max_child;
    }

    first[k] = std::move(e);
}

template<typename RandomIt,
    typename Compare = estd::less<typename estd::iterator_traits<RandomIt>::value_type> >
void heappush(RandomIt first, RandomIt last, Compare comp = Compare())
{
    auto k = last - first - 1; // k = last valid
    auto e = std::move(first[k]);

    while (k > 0 && comp(first[heap_parent(k)], e)) {
        first[k] = std::move(first[heap_parent(k)]);
        k = heap_parent(k);
    }

    first[k] = std::move(e);
}

}}
