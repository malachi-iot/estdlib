#pragma once

#include "iterator.h"

// In response to https://github.com/malachi-iot/estdlib/issues/11
// Undecided if we want to explicitly state forward_list here or
// if we hang it off traits.  I'd prefer to hang it off traits,
// but keeping it simpler to start with

namespace estd { namespace internal { namespace list {

// If we want an allocator (non intrusive), it will come along as part of traits
// though we might need an Impl for stateful allocators
template <class Node, class Data>
struct intrusive_traits
{
    // Get next ptr
    static Node* next(const Node* node)
    {
        return node->next();
    }

    // Re-assign next ptr
    static void next(Node* node, Node* v)
    {
        node->next(v);
    }

    // Experimental - seeing if we want to somehow grab data from
    // elsewhere.  Probably not.
    static Data& data(Node* node)
    {
        return *node;
    }

    static const Data& data(const Node* node)
    {
        return *node;
    }
};

// without tail
template <class T, class Traits = intrusive_traits<T> >
class intrusive_forward
{
    ESTD_CPP_STD_VALUE_TYPE(T)

    using node_type = pointer;

    node_type head_;

public:
    // Intrusive lists have a very different character for initialization
    // than their regular std counterparts
    ESTD_CPP_CONSTEXPR_RET intrusive_forward(pointer head = NULLPTR) :
        head_(head)
    {}

    using iterator = intrusive_iterator<T, Traits>;
    // DEBT: Still needs love, a pointer isn't gonna quite const up right here I think
    using const_iterator = intrusive_iterator<const T, Traits>;

    ESTD_CPP_CONSTEXPR_RET bool empty() const
    {
        return head_ == nullptr;
    }

    // TODO: clear() for intrusive takes on a different nature,
    // since we don't own the values per se.  Needs more attention
    void clear()
    {
        head_ = nullptr;
    }

    iterator begin()
    {
        return iterator(head_);
    }

    const_iterator begin() const
    {
        return const_iterator(head_);
    }

    const_iterator cbegin() const
    {
        return const_iterator(head_);
    }

    // range operator wants it this way.  Could do a fancy conversion from
    // null_iterator back to regular iterator too...
    ESTD_CPP_CONSTEXPR_RET iterator end() const
    {
        return iterator(nullptr);
    }

    ESTD_CPP_CONSTEXPR_RET null_iterator cend() const { return null_iterator(); }

    reference front()
    {
        return *head_;
    }

    iterator insert_after(const_iterator pos, reference value)
    {
        const_reference i = *pos;

        node_type n = Traits::next(&i);
        // DEBT: Casting away constness to conform with insert_after const_iterator signature.
        // intrusive is just that.  May dump const_iterator from signature completely since it's
        // kind of a lie.  Ponder the whole 'mutable' use case too.
        Traits::next(const_cast<pointer>(&i), &value);
        Traits::next(&value, n);

        return iterator(&value);
    }

    // Removes the element following pos.
    iterator erase_after(const_iterator pos)
    {
        const_reference i = *pos;

        node_type node_to_remove = Traits::next(&i);
        node_type node_after_node_to_remove = Traits::next(node_to_remove);
        Traits::next(const_cast<pointer>(&i), node_after_node_to_remove);

        return iterator(node_after_node_to_remove);
    }

    void push_front(reference value)
    {
        Traits::next(&value, head_);
        head_ = &value;
    }

    void pop_front()
    {
        head_ = Traits::next(head_);
    }
};

}}}