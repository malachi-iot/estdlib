/*
 * @file
 */
#pragma once

#include "internal/platform.h"
#include "internal/value_evaporator.h"

namespace estd {
template <class TPtr> struct pointer_traits;

template <class TAllocator>
struct allocator_traits;

template <class T>
class intrusive_forward_list;

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


///
/// @brief Not ready for prime time - fiddling with forward_list dependencies
///
template <class T>
class weak_ptr;


namespace internal {


struct deleter_base
{
    virtual void Deleter() {}
};


// shared_ptr itself sort of bypasses RIAA pattern, but this underlying control
// structure doesn't have to
template <class T>
struct shared_ptr_control_block2_base :
        deleter_base
{
    T* const shared;

    typedef uint16_t count_type;

    count_type shared_count;
    count_type weak_count;

    // FIX: this needs forward_list.h but we can't include that since it itself
    // includes memory.h
    //intrusive_forward_list<weak_ptr<T> > weak_list;

    typedef T managed_type;

    bool is_active; // only for use by 'master' - integrate as a bit field into shared_count possibly

    shared_ptr_control_block2_base(T* shared, bool is_active = true) :
        shared(shared),
        is_active(is_active)
    {
        shared_count = is_active ? 1 : 0;
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

    shared_ptr_control_block2(T* managed, TDeleter d, bool is_active = true) :
        base_type(managed, is_active),
        d(d)
    {}
};


///
/// @brief context parameter supporting deleter
///
/// can't quite get this one initialized...
///
template <class T, class TContext>
struct shared_ptr_control_block2<T, void (*)(T*, TContext)
    > :
    shared_ptr_control_block2_base<T>
{
    typedef shared_ptr_control_block2_base<T> base_type;

    typedef typename estd::remove_reference<TContext>::type context_type;

    //const void *deleter(TContext&);
    typedef void (*deleter_type)(T*, TContext);
    const deleter_type d;
    TContext& context;

    void Deleter() OVERRIDE
    {
        d(this->shared, context);
    }

    shared_ptr_control_block2(T* managed, deleter_type d, context_type& context) :
        base_type(managed),
        context(context),
        d(d)
    {}
};

// memory for 'shared' has been allocated in a way which the memory itself
// will be freed by a party outside shared_ptr, but we still need to explicitly
// call destructor when the ref count goes to 0
template <class T>
struct shared_ptr_control_block2<T, void> : shared_ptr_control_block2_base<T>
{
    typedef shared_ptr_control_block2_base<T> base_type;

    void Deleter() OVERRIDE
    {
        this->shared->~T();
    }

    shared_ptr_control_block2(T* managed, bool is_active = true) :
        base_type(managed, is_active)
    {}
};


}


template <class T>
class weak_ptr
{
    weak_ptr<T>* _next;

    weak_ptr<T>* next() const { return _next; }
    void next(weak_ptr<T>* assign) { _next = assign; }

    internal::shared_ptr_control_block2_base<T>* control;

public:
};


// std::shared_ptr sometimes leans on dynamic allocation for its control block
// obviously we don't want to do that, so experimenting with possibilities
namespace experimental {


template <class T, class TBase>
class shared_ptr2_base : public TBase
{
    typedef TBase base_type;

#ifdef UNIT_TESTING
public:
#endif
    typedef typename base_type::value_type control_type;

protected:
    struct deleter_tag {};

    T* stored;

    typedef typename control_type::count_type count_type;

    control_type& control() { return base_type::value(); }

    const control_type& control() const { return base_type::value(); }

    // run this only if we're actually connected to
    // a managed object
    void eval_delete()
    {
        if(--control().shared_count == 0)
        {
            control().Deleter();
        }
    }

public:
    count_type use_count() const
    {
        return base_type::is_active() ? control().shared_count : 0;
    }

    void reset()
    {
        if(!base_type::is_active()) return;

        eval_delete();
        base_type::deactivate();
    }

    typedef typename estd::remove_extent<T>::type element_type;

    element_type* get() const noexcept { return stored; }

    // TDeleter2 just because TDeleter sometimes is void in inheritance chain and
    // that irritates the compiler
    template <class TDeleter2>
    shared_ptr2_base(T* managed, TDeleter2 d) :
        base_type(managed, d),
        stored(managed)
    {}

    shared_ptr2_base(T* managed) :
        base_type(managed),
        stored(managed)
    {}

    shared_ptr2_base() {}


    template <class TDeleter2>
    shared_ptr2_base(TDeleter2 d, deleter_tag) :
        base_type(d)
    {}

    template <class TDeleter2, class TContext>
    shared_ptr2_base(TDeleter2 d, TContext& context, deleter_tag) :
        base_type(d, context)
    {}

    // still-ugly version of copy-increase-ref-counter. used primarily by non-master
    // shared_ptr to copy from master shared_ptr
    shared_ptr2_base(estd::internal::shared_ptr_control_block2_base<T>* master_shared_ptr) :
        base_type(master_shared_ptr)
    {
        if(master_shared_ptr && master_shared_ptr->is_active)
            master_shared_ptr->shared_count++;
    }

    ~shared_ptr2_base()
    {
        if(base_type::is_active()) eval_delete();
    }

    T* operator ->() const noexcept { return get(); }

    T& operator *() const noexcept { return *get(); }
};

}

namespace layer1 {

template <class TControlBlock,
          std::ptrdiff_t size = sizeof(typename TControlBlock::managed_type)>
struct shared_ptr_base :
        experimental::instance_provider<TControlBlock>
{
    typedef experimental::instance_provider<TControlBlock> base_type;
    typedef typename TControlBlock::managed_type managed_type;

    experimental::raw_instance_provider<managed_type, size> _provided;

    // FIX: clean up naming - getting 'value' collisions from multiple providers
    managed_type& provided() { return _provided.value(); }

    template <class TDeleter2>
    shared_ptr_base(TDeleter2 d) :
        base_type(&provided(), d, false)
    {}


    template <class TDeleter2, class TContext>
    shared_ptr_base(TDeleter2 d, TContext& context) :
        base_type(&provided(), d, context, false)
    {}

    // layer1 is special case where we've already allocated the buffer
    // but the object is not initialized, so send a false for is_active
    shared_ptr_base() :
        base_type(&provided(), false)
    {}

protected:
    bool is_active() const { return base_type::value().is_active; }
    void activate() { base_type::value().is_active = true; }
    void deactivate() { base_type::value().is_active = false; }
};


///
/// \brief Experimental
///
template <class T, class TDeleter = void,
          class TControlBlock = estd::internal::shared_ptr_control_block2<T, TDeleter>,
          std::ptrdiff_t size = sizeof(T)>
class shared_ptr : public experimental::shared_ptr2_base<T,
        shared_ptr_base<TControlBlock, size> >
{
    typedef experimental::shared_ptr2_base<T,
        shared_ptr_base<TControlBlock, size> > base_type;
    typedef typename base_type::deleter_tag deleter_tag;

public:
#ifdef FEATURE_CPP_DELETE_CTOR
    // this would duplicate the inline control structure. you don't want that
    shared_ptr(const shared_ptr&) = delete;
#endif

    shared_ptr() { this->stored = &(base_type::provided()); }


    template <class TDeleter2>
    shared_ptr(TDeleter2 d) :
        base_type(d, deleter_tag{})
    {}


    template <class TDeleter2, class TContext>
    shared_ptr(TDeleter2 d, TContext& context) :
        base_type(d, context, deleter_tag{})
    {}

    // non-standard call to initialize raw T
    // behavior is undefined if construct was called before
#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    void construct(TArgs&&...args)
    {
        new (&base_type::provided()) T(std::forward<TArgs>(args)...);
        base_type::activate();
        this->control().shared_count++;
    }
#endif
};


}

namespace layer2 {

template <class T, class TDeleter>
struct shared_ptr_base :
        experimental::instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> >
{
    typedef experimental::instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> > base_type;

    template <class TDeleter2>
    shared_ptr_base(T* managed, TDeleter2 d) :
        base_type(managed, d)
    {}

    shared_ptr_base(T* managed) : base_type(managed)
    {}

protected:
    bool is_active() const { return this->value().is_active; }
    void deactivate() { this->value().is_active = false; }
};


///
/// \brief Experimental
///
template <class T, class TDeleter = void>
class shared_ptr : public experimental::shared_ptr2_base<T,
        shared_ptr_base<T, TDeleter> >
{
    typedef experimental::shared_ptr2_base<T,
        shared_ptr_base<T, TDeleter> > base_type;

public:
#ifdef FEATURE_CPP_DELETE_CTOR
    // this would duplicate the inline control structure. you don't want that
    shared_ptr(const shared_ptr&) = delete;
#endif

    shared_ptr(T* managed) : base_type(managed)
    {
    }

    template <class TDeleter2>
    shared_ptr(T* managed, TDeleter2 d) :
        base_type(managed, d)
    {
    }

    /*
    // FIX: This is turning out to be kind of bad because it seems a copy-constructor
    // is attempted for shared_ptr2
    operator shared_ptr2<T>()
    {
        return shared_ptr2<T>(this->control());
    } */
};


}

namespace layer3 {


template <class T>
struct shared_ptr_base :
        experimental::instance_from_pointer_provider<estd::internal::shared_ptr_control_block2_base<T> >
{
    typedef experimental::instance_from_pointer_provider<
        estd::internal::shared_ptr_control_block2_base<T> > base_type;

protected:
    bool is_active() const { return this->value_ptr() != NULLPTR; }
    void deactivate() { this->value_ptr(NULLPTR); }

    shared_ptr_base(estd::internal::shared_ptr_control_block2_base<T>* assign_from) :
        base_type(assign_from)
    {

    }
};


///
/// \brief Experimental
///
template <class T>
class shared_ptr : public experimental::shared_ptr2_base<T,
        shared_ptr_base<T> >
{
    typedef experimental::shared_ptr2_base<T, shared_ptr_base<T> > base_type;

public:
    // for when copying/referencing a master provider.  they are always allocated
    // if we didn't distinctively need master type here then we could genericize and
    // share code with the shared_ptr& copy_from constructor below
    template <class TDeleter>
    shared_ptr(experimental::instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> >&
               copy_from) :
        base_type(copy_from.value().is_active ? &copy_from.value() : NULLPTR)
    {}

    template <class TControlBlock>
    shared_ptr(layer1::shared_ptr_base<TControlBlock>& copy_from) :
        base_type(copy_from.value().is_active ? &copy_from.value() : NULLPTR)
    {

    }

    // value_ptr might be null here if copy_from isn't tracking anything
    explicit shared_ptr(shared_ptr& copy_from) : base_type(copy_from.value_ptr()) {}

    //template <class TDeleter>
    //shared_ptr& operator=(experimental::instance_provider<estd::internal::shared_ptr_control_block2<T, TDeleter> >&
    template <class TSharedPtr>
    shared_ptr& operator=(TSharedPtr&
                         copy_from)
    {
        this->reset();
        new (this) shared_ptr(copy_from);
        return *this;
    }
};


}




}
