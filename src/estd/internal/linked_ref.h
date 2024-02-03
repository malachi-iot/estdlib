#pragma once

#include "list/intrusive_forward.h"

// Relative of shared_ptr.  Uses a few tricks to maintain a circular list/queue
// of available shared/weak references.  Slightly computationally expensive, but
// very memory frugal

namespace estd { namespace internal {

// DEBT: Might need to be a stateful impl, but we want to avoid that
template <class T>
struct linked_ref_traits
{
    static bool null(T* v)
    {
        return v == nullptr;
    }

    static void destroy(T*) {}
};

template <class T, class Traits = linked_ref_traits<T> >
class linked_ref
{
    using list_type = list::intrusive_forward<linked_ref>;
    using iterator = typename list_type::iterator;
    //using const_iterator = typename list_type::const_iterator;
    // FIX: const_iterator needs const on value itself, not whole iterator
    using const_iterator = typename list_type::const_iterator;

    ESTD_CPP_STD_VALUE_TYPE(T)

    const value_type value_;
    // Use forward_node_base_base once all this settles down
    linked_ref* next_;

public:
    linked_ref(value_type value) :
        value_(value),
        next_(this)
    {
    }

    linked_ref(linked_ref* attach_to) :
        value_(attach_to->value_)
    {
        list_type l(attach_to->next());

        /*
        iterator i(attach_to);

        //for(;;)
        {
            linked_ref& current = *i;

            if(current.next() == attach_to)
            {

            }
        }   */
    }
    
    linked_ref* next() const { return next_; }
    void next(linked_ref* v) { next_ = v; }
    linked_ref& data() const { return *this; }

    int count_shared()
    {
        int count = 1;  // ourself

        list_type l(this);

        const_iterator i = l.begin();

        // FIX: Yeah, we don't expect to ever see a null iterator here.
        // linked_ref goes in a circle
        while(++i != list::null_iterator{})
        {
            // TODO: filter by null/non-weak
            linked_ref& current = *i;

            if(current == *this) return count;

            //if(!Traits::null(&current))
            ++count;
        }

        return count;
    }
};

}}