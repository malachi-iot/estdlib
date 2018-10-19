#pragma once

#include "iterators/list.h"
#include "traits/list_node.h"
#include "forward_list.h"
#include "internal/reverse_list.h"

namespace estd {

namespace internal {

// TODO: naming list_base because there's a namespace already called estd::internal::list.
// Would like to resolve that and change this back to 'list' again
template<class T, class TNode = T,
         class TAllocator = experimental_std_allocator<TNode>,
         class TNodeTraits = node_traits<TNode, TAllocator, nothing_allocator<T> > >
class list_base :
        public internal::reverse_list<
            internal::forward_list_base<
                TNodeTraits,
                internal::list::BidirectionalIterator<T, TNodeTraits> > >
{
    typedef internal::reverse_list<
                internal::forward_list_base<
                    TNodeTraits,
                    internal::list::BidirectionalIterator<T, TNodeTraits> > > base_t;

    typedef typename base_t::allocator_t allocator_t;
    typedef typename base_t::value_type value_type;
    typedef value_type& reference;
    typedef TNodeTraits node_traits_t;
    typedef typename base_t::node_handle node_handle;
    typedef typename base_t::iterator iterator;
    typedef typename base_t::const_iterator const_iterator;
    typedef typename base_t::node_type node_type;
    typedef typename base_t::nv_reference nv_reference;

    // FIX: gonna need something better than this
    static CONSTEXPR node_handle before_beginning_node() { return base_t::after_end_node(); }


    void push_back(node_handle new_back)
    {
        // TODO: assert that if m_front is invalid, so is m_back

        // if we do have a back node, be sure to
        // point it at the incoming new back node
        if(base_t::m_back != base_t::after_end_node())
            base_t::set_next(base_t::m_back, new_back);

        // if we had no front node yet, now we do
        if(base_t::m_front == before_beginning_node())
            base_t::m_front = new_back;

        base_t::set_back(new_back);
    }

    void push_front(node_handle new_front)
    {
        // TODO: assert that if m_front is invalid, so is m_back

        // only update m_front->prev if it actually is present
        if(base_t::m_front != before_beginning_node())
            // m_front will only have an invalid ptr for prev, so we
            // can easily overwrite that
            base_t::set_prev(base_t::m_front, new_front);

        // if we have no back node yet, now we do
        if(base_t::m_back == base_t::after_end_node())
            base_t::m_back = new_front;

        base_t::set_front(new_front);
    }

public:
    list_base(allocator_t* allocator = NULLPTR) : base_t(allocator) {}

    void push_front(const T& value)
    {
        push_front(base_t::traits.allocate(value));
    }


    void push_back(const T& value)
    {
        push_back(base_t::traits.allocate(value));
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
    void push_front(T&& value)
    {
        node_handle new_front = base_t::traits.alloc_move(std::move(value));

        push_front(new_front);
    }

    void push_back(T&& value)
    {
        push_back(base_t::traits.alloc_move(std::move(value)));
    }
#endif

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
    void emplace_front(TArgs&&...args)
    {
        node_handle new_front = base_t::traits.alloc_emplace(std::forward<TArgs...>(args)...);

        push_front(new_front);
    }
#endif


    iterator insert(const_iterator pos, const T& value)
    {
        // special case, if inserting at front, fall through to our regular
        // push_front
        if(pos == base_t::begin())
        {
            push_front(value);
            return base_t::begin();
        }

        node_handle new_node = base_t::traits.allocate(value);

        node_handle node_to_insert_on = pos.node(); // or 'before', if you like
        node_handle node_to_insert_after = base_t::prev(node_to_insert_on);

        // splice new node before 'pos' node
        base_t::set_next(new_node, node_to_insert_on);
        // update next pointer of node before 'pos' node to now point to new node
        base_t::set_next(node_to_insert_after, new_node);
        // update prev pointer of new node to point to node just preceding 'pos'
        base_t::set_prev(new_node, node_to_insert_after);
        // update prev pointer of old 'pos' node to now pointer to new node
        base_t::set_prev(node_to_insert_on, new_node);

        return iterator(new_node, base_t::traits);
    }
};

}

// Inspired by ETL
template <class T>
class intrusive_list : public internal::list_base<T> { };

}
