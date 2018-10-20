/**
 *  @file
 */
#pragma once

#include "internal/platform.h"
//#include "memory.h"
#include "traits/list_node.h"
#include "internal/list_node.h"
#include "iterators/list.h"
#include "utility.h"

namespace estd {


// ugly one to try to get inline value working
template <class TValue, class TNodeAllocator>
struct node_traits<experimental::ValueNode<TValue, experimental::forward_node_base>,
        TNodeAllocator,
        nothing_allocator<TValue> > :
        public inlinevalue_node_traits_new_base<
            experimental::ValueNode<TValue, experimental::forward_node_base>,
            TNodeAllocator,
            nothing_allocator<TValue> >
{

};

// putting root forward_list inside internal namespace for now, because it's so
// fluid in its allocation capabilities that it's a bit confusing to leave it
// as regular estd::forward_list - the crux of the problem being what implicit
// behavior do we want estd::forward_list to have?  I was defaulting it to intrusive,
// but that's the least-compatible option with std::forward_list
namespace internal {


// TODO: Due to the requisite deviation for Allocator to take a <T>,
// consider adding Allocator (<T>) to forward list and yanking all allocator
// direct references out of TNodeTraits.  TNodeTraits will have to rely on
// incoming templatized allocator calls.  One possibility is TNodeTrait can
// present a 'default' allocator , though this might conflict with a default of
// std::allocator<T> specialization, but that itself is deprecated for C++17
template<class T, class TNode = T,
        class TNodeAllocator = experimental_std_allocator<TNode>,
        class TNodeTraits = node_traits<TNode, TNodeAllocator, nothing_allocator<T> > >
class forward_list : public forward_list_base<TNodeTraits, internal::list::ForwardIterator<T, TNodeTraits&> >
{
public:
    typedef internal::forward_list_base<TNodeTraits, internal::list::ForwardIterator<T, TNodeTraits&> > base_t;
    typedef T value_type;
    typedef value_type& reference;
    typedef TNodeTraits node_traits_t;
    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;
    typedef typename base_t::node_allocator_t node_allocator_t;
    //typedef typename node_traits_t::template node_allocator_t<value_type> node_allocator_t;
    typedef node_allocator_t allocator_t;
    typedef typename base_t::node_type node_type;

    typedef typename base_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

    typedef allocator_traits<allocator_t> allocator_traits_t;

    node_traits_t& get_traits() { return base_t::get_traits(); }
    const node_traits_t& get_traits() const { return base_t::get_traits(); }

protected:

    typedef typename base_t::node_handle node_handle;
    //typedef typename node_traits_t::node_handle node_handle;

    static CONSTEXPR node_handle after_end_node() { return base_t::after_end_node(); }

    // FIX: before_beginning_node() needs work, not gonna really work like this
    static CONSTEXPR node_handle before_beginning_node() { return after_end_node(); }

    node_type& alloc_lock(node_handle& to_lock)
    {
        return base_t::alloc_lock(to_lock);
    }

    void alloc_unlock(node_handle& to_unlock)
    {
        base_t::alloc_unlock(to_unlock);
    }

    node_handle next(node_handle from)
    {
        return base_t::next(from);
    }

    void set_next(node_handle _node, node_handle next)
    {
        base_t::set_next(_node, next);
    }

    /*
    void destroy(node_handle h)
    {
        get_traits().destroy(h);
    } */

public:
    //forward_list(node_traits_t& traits)
    forward_list(allocator_t* allocator = NULLPTR) :
            base_t(allocator) {}

    // NOTE: traits eventually should become stateless and
    // perhaps have allocator passed into it, in which case
    // this constructor will be obsolete
    forward_list(node_traits_t& traits) :
        base_t(traits) {}

    void pop_front()
    {
#ifdef DEBUG
        // undefined behavior if list is empty, but let's put some asserts in here
#endif
        node_handle old = base_t::m_front;

        base_t::m_front = next(base_t::m_front);

        get_traits().deallocate(old);
    }

    void push_front(nv_reference value)
    {
        base_t::set_front(get_traits().allocate(value));
    }

#ifdef FEATURE_CPP_MOVESEMANTIC

    void push_front(value_type&& value)
    {
        base_t::set_front(get_traits().alloc_move(std::move(value)));
    }

#endif


    iterator insert_after(const_iterator pos, nv_reference value)
    {
        node_handle node_to_insert_after = pos.node();
        node_handle old_next_node = next(node_to_insert_after);
        node_handle node_pointing_to_value = base_t::traits.allocate(value);

        set_next(node_to_insert_after, node_pointing_to_value);
        set_next(node_pointing_to_value, old_next_node);

        return iterator(node_pointing_to_value, base_t::traits);
    }

    iterator erase_after(const_iterator pos)
    {
        node_handle node_to_erase_after = pos.node();
        node_handle node_to_erase = next(node_to_erase_after);
        node_handle node_following_erased = next(node_to_erase);

        set_next(node_to_erase_after, node_following_erased);

        return iterator(node_following_erased, base_t::traits);
    }

#ifdef FEATURE_CPP_VARIADIC

    // a bit tricky because we have 4 allocation possibilities:
    // 'intrusive' where node data itself is a resident of value_type itself, externally allocated
    // 'non-intrusive' where node data is managed via a reference (RefNode) - this right now
    //                 only works with externally-allocated refs.  Ultimately this would split
    //                 into RefNode and RefNodeManaged where the latter allocs and deletes
    //                 its own value_type
    // 'semi-intrusive' where really it is intrusive but in reverse, forward_list allocates
    //                  both the node and value_type together as part of the node.  In effect
    //                  keeps copies of items placed into array
    template<class... TArgs>
    node_handle emplace_front(TArgs&& ... args)
    {
        static_assert(node_traits_t::can_emplace(), "This node allocator cannot emplace");

        node_handle h = base_t::traits.alloc_emplace(std::forward<TArgs>(args)...);

        base_t::set_front(h);

        // deviates from std::forward_list in that that returns
        // a reference, but we return a handle which will need
        // a lock()
        // TODO: Use some form of accessor here instead, more useful than a raw handle
        return h;
    }

#endif


    // deviates from std C++ in that this optionally shall remove the first item found
    // in the list, rather than all items matching
    // TODO: change remove/remove_if to use iterators more directly to avoid this broken before_beginning_node magic
    template<class UnaryPredicate>
    void remove_if(UnaryPredicate p, bool first_only = false)
    {
        node_handle current = base_t::m_front;
        node_handle previous = before_beginning_node();

        while (current != after_end_node())
        {
            node_handle _next = next(current);
            node_type& current_locked = alloc_lock(current);

            bool matched = p(current_locked);

            alloc_unlock(current);

            if (matched)
            {
                // If we match but there's no previous node
                if (previous == before_beginning_node())
                    // then instead of splicing, we are replacing the front node
                    base_t::m_front = _next;
                else
                    set_next(previous, _next);

                if (first_only) return;
            }

            previous = current;
            current = _next;
        }
    }

    // deviates from std C++ in that this optionally shall remove the first item found
    // in the list, rather than all items matching
    //NOTE: if C++03 will handle it, lean on remove_if instead of duplicating the code
    //TODO: Unlikely C++03 wil handle that, so instead do a #define to wrap up predicate
    // portion and fold into one remove_internal which is called differently depending
    // on C++03 or C++11
    void remove(nv_reference value, bool first_only = false)
    {
#ifdef FEATURE_CPP_LAMBDA
        remove_if([&value](nv_reference compare_to) {
            return value == compare_to;
        }, first_only);
#else
        node_handle current = base_t::m_front;
        node_handle previous = before_beginning_node();

        while(current != after_end_node())
        {
            node_handle _next = next(current);
            node_type& current_locked = alloc_lock(current);

            bool matched = current_locked == value;

            alloc_unlock(current);

            if(matched)
            {
                // If we match but there's no previous node
                if(previous == node_traits_t::null_node())
                    // then instead of splicing, we are replacing the front node
                    base_t::m_front = _next;
                else
                    set_next(previous, next(current));

                if(first_only) return;
            }

            previous = current;
            current = _next;
        }
#endif
    }
};

///
/// \tparam T
/// \tparam TNode
/// \tparam TNodeAllocator
/// \tparam TNodeTraits
/// \remarks UNTESTED!
template<class T, class TNode = T,
        class TNodeAllocator = experimental_std_allocator<TNode>,
        class TNodeTraits = node_traits<TNode, TNodeAllocator, nothing_allocator<T> > >
class forward_list_with_back : public forward_list_with_back_base<
        TNodeTraits, internal::list::ForwardIterator<T, TNodeTraits&>,
        forward_list<T, TNode, TNodeAllocator, TNodeTraits > >
{
    typedef forward_list_with_back_base<
            TNodeTraits, internal::list::ForwardIterator<T, TNodeTraits&>,
            forward_list<T, TNode, TNodeAllocator, TNodeTraits> > base_type;
};

}

// Inspired by ETL library
template <class T>
class intrusive_forward_list : public internal::forward_list<T>  { };

template <class T>
class intrusive_forward_list_with_back : public internal::forward_list_with_back<T>  { };



}
