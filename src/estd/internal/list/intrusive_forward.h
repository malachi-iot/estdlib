#pragma once

#include "traits.h"
#include "base.h"

// In response to https://github.com/malachi-iot/estdlib/issues/11
// Undecided if we want to explicitly state forward_list here or
// if we hang it off traits.  I'd prefer to hang it off traits,
// but keeping it simpler to start with

namespace estd { namespace internal { namespace list {

// without tail
template <class T, class Traits = intrusive_traits<T> >
class intrusive_forward : public head_base<T, Traits>
{
    using base_type = head_base<T, Traits>;

protected:
    ESTD_CPP_STD_VALUE_TYPE(T)

    using node_type = pointer;

public:
    // Intrusive lists have a very different character for initialization
    // than their regular std counterparts
    EXPLICIT ESTD_CPP_CONSTEXPR_RET intrusive_forward(pointer head = NULLPTR) :
        base_type(head)
    {}

    using typename base_type::iterator;
    using typename base_type::const_iterator;

    // TODO: clear() for intrusive takes on a different nature,
    // since we don't own the values per se.  Needs more attention
    void clear()
    {
        base_type::head_ = nullptr;
    }

    static iterator insert_after(const_iterator pos, reference value)
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
    static iterator erase_after(const_iterator pos)
    {
        const_reference i = *pos;

        node_type node_to_remove = Traits::next(&i);
        node_type node_after_node_to_remove = Traits::next(node_to_remove);
        Traits::next(const_cast<pointer>(&i), node_after_node_to_remove);

        return iterator(node_after_node_to_remove);
    }

    // Non standard operation to combine erase/insert into one
    static iterator replace_after(const_iterator pos, reference value)
    {
        // DEBT: Combine algo to speed it up

        erase_after(pos);
        return insert_after(pos, value);
    }

    void push_front(reference value)
    {
        Traits::next(&value, base_type::head_);
        base_type::head_ = &value;
    }

    void pop_front()
    {
        base_type::head_ = Traits::next(base_type::head_);
    }
};

template <class T, class Traits = intrusive_traits<T> >
class circular_intrusive_forward : public intrusive_forward<T, Traits>
{
    using base_type = intrusive_forward<T, Traits>;
    using typename base_type::node_type;

public:
    using typename base_type::iterator;
    using typename base_type::pointer;

    // Intrusive lists have a very different character for initialization
    // than their regular std counterparts
    EXPLICIT ESTD_CPP_CONSTEXPR_RET circular_intrusive_forward(pointer head = NULLPTR) :
        base_type(head)
    {}

    // TODO: Do before_begin here - which doesn't make sense for a regular intrusive list
    // but does here
    iterator before_begin()
    {
        node_type current = base_type::head_;

        while(Traits::next(current) != base_type::head_)
            current = Traits::next(current);

        return iterator(current);
    }
};


}}}