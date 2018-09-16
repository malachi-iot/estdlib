#pragma once

#include "internal/platform.h"
#include "internal/value_evaporator.h"

namespace estd {
template <class TPtr> struct pointer_traits;

template <class TAllocator>
struct allocator_traits;

}

#include "allocators/generic.h"
#ifdef FEATURE_STD_MEMORY
#include <memory>
#endif

#include "cstdint.h"

namespace estd {

template <class T> struct pointer_traits<T*>
{
    typedef std::ptrdiff_t difference_type;
};


namespace internal {

template <class T>
struct shared_ptr_control_block
{
    T* shared;
    // seems very much like we'd want a polymorphic (or similar) allocator here
    union
    {
        struct
        {
            uint16_t shared_count : 7;
            uint16_t weak_count : 7;
            // indicates during inline mode if this control block is active
            // think of it as the 'master' active bit, even though subordinates
            // are independently active or not
            uint16_t is_active : 1;
            uint16_t is_inline : 1;
        };

        uint16_t raw;
    };
};


struct deleter_base
{
    virtual void Deleter() {}
};


template <class T>
struct shared_ptr_control_block2_base :
        deleter_base
{
    T* shared;

    uint16_t shared_count;
    uint16_t weak_count;
    bool is_active; // only for use by 'master' - integrate as a bit field into shared_count possibly

    shared_ptr_control_block2_base()
    {
        shared_count = 0;
        weak_count = 0;
    }

    shared_ptr_control_block2_base(T* shared) :
        shared(shared)
    {
        shared_count = 1;
        weak_count = 0;
    }
};


// memory for 'shared' has been allocated dynamically and we need
// delete to be destructor and deallocator, as delete traditionally does
template <class T, class TDeleter>
struct shared_ptr_control_block2 :
        shared_ptr_control_block2_base<T>
{
    typedef shared_ptr_control_block2_base<T> base_type;

    TDeleter d;

    void Deleter() OVERRIDE
    {
        d(this->shared);
    }

    shared_ptr_control_block2(T* managed, TDeleter d) :
        base_type(managed),
        d(d)
    {}
};

// memory for 'shared' has been allocated in a way which the memory itself
// will be freed, but we still need to explicitly call destructor when the
// ref count goes to 0
template <class T>
struct shared_ptr_control_block2<T, void> : shared_ptr_control_block2_base<T>
{
    void Deleter() OVERRIDE
    {
        this->shared->~T();
    }
};



}

// std::shared_ptr sometimes leans on dynamic allocation for its control block
// obviously we don't want to do that, so experimenting with possibilities
namespace experimental {

template <class T, class TDeleter, class TBase>
class shared_ptr2_base : public TBase
{
    // not using this yet until I better grasp the use cases for the stored ptr feature
    T* stored;

    typedef TBase base_type;

    //typedef estd::internal::shared_ptr_control_block2<T, TDeleter> control_type;
    typedef typename base_type::value_type control_type;

protected:
    control_type& control() { return base_type::value(); }

    const control_type& control() const { return base_type::value(); }

    long use_count()
    {
        return control().shared_count;
    }

    // run this only if we're actually connected to
    // a managed object
    void eval_delete()
    {
        if(--control().shared_count == 0)
        {
            control().Deleter();
        }
    }

    // internal one, doesn't decouple *this
    void reset()
    {
        eval_delete();
        //control().shared_count--;
    }

public:
    typedef typename estd::remove_extent<T>::type element_type;

    element_type* get() const noexcept { return stored; }

    // TDeleter2 just because TDeleter sometimes is void in inheritance chain and
    // that irritates the compiler
    template <class TDeleter2>
    shared_ptr2_base(T* managed, TDeleter2 d) :
        base_type(managed, d),
        stored(managed)
    {}

    // still-ugly version of copy-increase-ref-counter. used primarily by non-master
    // shared_ptr
    shared_ptr2_base(estd::internal::shared_ptr_control_block2_base<T>& temp) :
        base_type(&temp)
    {
        temp.shared_count++;
    }
};

template <class T>
class shared_ptr2 : public shared_ptr2_base<T, void,
        instance_from_pointer_provider<estd::internal::shared_ptr_control_block2_base<T> > >
{
    typedef shared_ptr2_base<T, void,
        instance_from_pointer_provider<estd::internal::shared_ptr_control_block2_base<T> > > base_type;

protected:
    bool is_active() const { return this->value_ptr() != NULLPTR; }

public:
    long use_count() const
    {
        return is_active() ? base_type::use_count() : 0;
    }

    void reset()
    {
        base_type::reset();
        this->value_ptr(NULLPTR);
    }

    template <class TDeleter>
    shared_ptr2(instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> >& temp) :
        base_type(temp.value())
    {}

    //shared_ptr2(estd::internal::shared_ptr_control_block2_base<T>& temp) : base_type(temp) {}

    explicit shared_ptr2(shared_ptr2& copy_from) : base_type(copy_from.value()) {}

    ~shared_ptr2()
    {
        if(is_active()) base_type::eval_delete();
    }
};

template <class T, class TDeleter>
class shared_ptr2_master : public shared_ptr2_base<T, TDeleter,
        instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> > >
{
    typedef shared_ptr2_base<T, TDeleter,
        instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> > > base_type;

protected:
    bool is_active() const { return this->control().is_active; }

public:
    long use_count() const
    {
        return is_active() ? base_type::use_count() : 0;
    }

    void reset()
    {
        base_type::reset();
        this->control().is_active = false;
    }

    shared_ptr2_master(T* managed, TDeleter d) :
        base_type(managed, d)
    {
        this->control().is_active = true;
    }

    /*
    // FIX: This is turning out to be kind of bad because it seems a copy-constructor
    // is attempted for shared_ptr2
    operator shared_ptr2<T>()
    {
        return shared_ptr2<T>(this->control());
    } */

    ~shared_ptr2_master()
    {
        if(this->control().is_active) base_type::eval_delete();
    }
};

// TODO: use a memory pool to allocate a shared_ptr_control_block
template <class T, class TDeleter = void>
class shared_ptr
{
    T* stored;

    typedef estd::internal::shared_ptr_control_block<T> control_type;

    // since inline is always bigger than control_ptr, we're going to presume
    // is_inline is intact always
    union
    {
        control_type* control_ptr;
        // expanding size of shared_ptr from std a bit, but we avoid allocation.
        // also beware that the weak_count and shared_count only go to 127
        control_type control_inline;
    };

    control_type* control() const
    {
        if(control_inline.is_inline)
            return control_inline.is_active ? &control_inline : NULLPTR;
        else
            return control_ptr;
    }



public:
    typedef typename estd::remove_extent<T>::type element_type;

    template <class Y>
    explicit shared_ptr(Y* ptr) :
        stored(ptr)
    {
        control_inline.raw = 0;
        control_inline.is_inline = true;
        control_inline.is_active = true;
        control()->shared = ptr;
    }

    template <class Y>
    shared_ptr(const shared_ptr<Y>& r) noexcept
    {
        control_inline.raw = 0; // is_inline = false
        control_ptr = r.control();
    }

    element_type* get() const noexcept { return stored; }

    long use_count() const noexcept
    {
        return control()->shared_count;
    }

    void reset() noexcept
    {
        control_type* c = control();
        if(!c) return;

        if(--c->shared_count == 0)
        {
            // destroy object
        }

        // if not inline, we can simply blast control_ptr
        // to decouple and not manage anymore
        if(!control_inline.is_inline)
        {
            control_ptr = NULLPTR;
        }
        else
        {
            control_inline.is_active = false;
        }
    }
};

}



}
