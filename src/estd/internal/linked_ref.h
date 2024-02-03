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
    static T* null() { return nullptr; }
    static bool null(T* v)
    {
        return v == nullptr;
    }

    static void destroy(T*) {}
};

template <class T, class Traits = linked_ref_traits<T> >
class linked_ref
{
    using list_type = list::circular_intrusive_forward<linked_ref>;
    using iterator = typename list_type::iterator;
    using const_iterator = typename list_type::const_iterator;

    ESTD_CPP_STD_VALUE_TYPE(T)

    const value_type value_;
    // Use forward_node_base_base once all this settles down
    linked_ref* next_;

    constexpr bool is_shared() const
    {
        return Traits::null(value_) == false;
    }

    constexpr bool is_weak() const
    {
        return Traits::null(value_) == true;
    }

    // for weak one only
    linked_ref() :
        value_(Traits::null())
    {

    }

public:
    constexpr explicit linked_ref(value_type value) :
        value_(value),
        next_(this)
    {
    }

    explicit linked_ref(const linked_ref& attach_to) :
        value_(attach_to.value_)
    {
        // DEBT: Do a const_cast here, maybe mutable?
        // we are forced to do this so as to conform to regular copy constructor signature
        list_type l((linked_ref*)&attach_to);

        l.insert_after(const_iterator(&attach_to), *this);

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

    linked_ref(linked_ref&& attach_to) noexcept :
        value_(attach_to.value_)
    {
        // NOTE: Sad truth is move operation is pretty slow, since we have to remove ourselves
        // from a forward list necessitating a full loop through.  Naturally doubly linked list
        // is superior here, but that's
        // 1. not implemented yet
        // 2. increases our footprint by ~33%
        list_type l(&attach_to);
        const_iterator pos(l.before_begin().current_);

        // Not quite ready yet
        l.replace_after(pos, *this);
    }

    ~linked_ref()
    {
        //  only shared flavor link to themselves.  weak flavor almost do but they get unlinked
        // before they get the chance
        if(this == next_) //&& is_shared())
        {
            Traits::destruct(value_);
        }
        else
        {
            // go in a complete circle and find starting point just before us
            linked_ref* n = this;
            int shared_counter = 0;
            int weak_counter = 0;

            while(n->next() != this)
            {
                n = n->next();

                if(n->is_shared())
                    ++shared_counter;
                else
                    ++weak_counter;
            }

            // found it, just before us
            list_type l(n);

            // delete next element after element just before us...
            // which is ... us!
            l.erase_after(n);

            if(shared_counter == 0)
            {
                // we're the last shared
                Traits::destruct(value_);

                // now unlink all the weak ones
                if(weak_counter > 0)
                {
                    while(n != nullptr)
                    {
                        linked_ref* next2 = n->next();
                        n->next_ = nullptr;
                        n = next2;
                    }
                }
            }
        }
    }
    
    linked_ref* next() const { return next_; }
    void next(linked_ref* v) { next_ = v; }
    //linked_ref& data() const { return *this; }

    linked_ref make_weak() const
    {
        linked_ref lr;

        list_type l(&lr);

        l.insert_after(const_iterator(this), *this);

        return lr;
    }

    int count_shared()
    {
        int count = 1;  // ourself

        list_type l(this);

        const_iterator i = l.cbegin();

        // FIX: Yeah, we don't expect to ever see a null iterator here.
        // linked_ref goes in a circle
        while(++i != list::null_iterator{})
        {
            // TODO: filter by null/non-weak
            const linked_ref& current = *i;

            if(&current == this) return count;

            //if(!Traits::null(&current))
            ++count;
        }

        return count;
    }
};

}}