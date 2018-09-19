/*
 * @file
 */
#pragma once

#include "internal/platform.h"
#include "internal/value_evaporator.h"

// turns on or off the 'stored' pointer feature of a shared_ptr
//#define FEATURE_ESTD_SHARED_PTR_ALIAS
#define FEATURE_ESTD_WEAK_PTR

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
template <class T, typename TCount = uint16_t>
struct shared_ptr_control_block :
        deleter_base
{
    T* const shared;

    typedef TCount count_type;

    count_type shared_count;
#ifdef FEATURE_ESTD_WEAK_PTR
    count_type weak_count;

    // FIX: this needs forward_list.h but we can't include that since it itself
    // includes memory.h
    //intrusive_forward_list<weak_ptr<T> > weak_list;
#endif

    typedef T managed_type;

    bool is_active; // only for use by 'master' - integrate as a bit field into shared_count possibly

    shared_ptr_control_block(T* shared, bool is_active = true) :
        shared(shared),
        is_active(is_active)
    {
        shared_count = is_active ? 1 : 0;
#ifdef FEATURE_ESTD_WEAK_PTR
        weak_count = 0;
#endif
    }
};


// memory for 'shared' has been allocated dynamically and we need
// delete to be destructor and deallocator, as delete traditionally does
template <class T, class TDeleter>
struct shared_ptr_inline_deleter_control_block :
        shared_ptr_control_block<T>
{
    typedef shared_ptr_control_block<T> base_type;

    TDeleter d;

    void Deleter() OVERRIDE
    {
        d(this->shared);
    }

    shared_ptr_inline_deleter_control_block(T* managed, TDeleter d, bool is_active = true) :
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
struct shared_ptr_inline_deleter_control_block<T, void (*)(T*, TContext)
    > :
    shared_ptr_control_block<T>
{
    typedef shared_ptr_control_block<T> base_type;

    typedef typename estd::remove_reference<TContext>::type context_type;

    //const void *deleter(TContext&);
    typedef void (*deleter_type)(T*, TContext);
    const deleter_type d;
    TContext& context;

    void Deleter() OVERRIDE
    {
        d(this->shared, context);
    }

    shared_ptr_inline_deleter_control_block(T* managed, deleter_type d, context_type& context) :
        base_type(managed),
        context(context),
        d(d)
    {}
};

// memory for 'shared' has been allocated in a way which the memory itself
// will be freed by a party outside shared_ptr, but we still need to wire up
// destructor call for when ref count goes to 0
// NOTE: this might be better served as the default deleter in shared_ptr_control_block itself
// we might save some space on virtual function tables
template <class T>
struct shared_ptr_inline_deleter_control_block<T, void> : shared_ptr_control_block<T>
{
    typedef shared_ptr_control_block<T> base_type;

    void Deleter() OVERRIDE
    {
        this->shared->~T();
    }

    shared_ptr_inline_deleter_control_block(T* managed, bool is_active = true) :
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

    internal::shared_ptr_control_block<T>* control;

public:
};

namespace layer3 {

template <class T>
class shared_ptr;

}

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

#ifdef FEATURE_ESTD_SHARED_PTR_ALIAS
    T* stored;
#endif

    typedef typename control_type::count_type count_type;

    control_type& control() { return base_type::value(); }

    const control_type& control() const { return base_type::value(); }

    ///
    /// \brief eval_delete evaluates whether we should delete and if so, do it
    ///
    /// run this only if we're actually connected to a managed object
    ///
    /// \param with_deactivate
    ///
    void eval_delete(bool with_deactivate = false)
    {
        // acquire reference to c here, just incase deactivation
        // decouples us from it
        control_type& c = control();

        // Doing inside eval_delete due to semi-recursive nature of
        // eval_delete call during special deleters (i.e. memory pool)
        if(with_deactivate)
            // when using layer3, for example, c is still valid here
            // even though underlying value* has been decoupled
            base_type::deactivate();

        if(--c.shared_count == 0)
        {
            c.Deleter();
        }
    }

public:
    // untested
    bool owner_before(const shared_ptr2_base& other) const
    {
        return &control() < &other.control();
    }

    count_type use_count() const
    {
        return base_type::is_active() ? control().shared_count : 0;
    }

    ///
    /// \brief unlinks this shared_ptr from any managed object
    ///
    /// if use_count ends up 0, then also invokes deleter
    ///
    void reset()
    {
        if(base_type::is_active()) eval_delete(true);
    }

    typedef typename estd::remove_extent<T>::type element_type;

    element_type* get() const noexcept
    {
#ifdef FEATURE_ESTD_SHARED_PTR_ALIAS
        return stored;
#else
        return control().shared;
#endif
    }

    // TDeleter2 just because TDeleter sometimes is void in inheritance chain and
    // that irritates the compiler
    template <class TDeleter2>
    shared_ptr2_base(T* managed, TDeleter2 d) :
        base_type(managed, d)
#ifdef FEATURE_ESTD_SHARED_PTR_ALIAS
        ,
        stored(managed)
#endif
    {}

    shared_ptr2_base(T* managed) :
        base_type(managed)
#ifdef FEATURE_ESTD_SHARED_PTR_ALIAS
        ,
        stored(managed)
#endif
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
    shared_ptr2_base(estd::internal::shared_ptr_control_block<T>* master_shared_ptr) :
        base_type(master_shared_ptr)
    {
        if(master_shared_ptr && master_shared_ptr->is_active)
            master_shared_ptr->shared_count++;
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    shared_ptr2_base(shared_ptr2_base&& move_from) :
        base_type(std::move(move_from))
    {

    }
#endif

    ~shared_ptr2_base()
    {
        // we *could* specify deactivate, but memory is about to vanish
        // anyway so nobody cares about that status
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
          class TControlBlock = estd::internal::shared_ptr_inline_deleter_control_block<T, TDeleter>,
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


#ifdef FEATURE_ESTD_SHARED_PTR_ALIAS
    shared_ptr() { this->stored = &(base_type::provided()); }
#else
    shared_ptr() {}
#endif


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

    /*
    operator layer3::shared_ptr<T>()
    {
        return layer3::shared_ptr<T>(*this);
    } */
};


}

namespace layer2 {

template <class T, class TDeleter>
struct shared_ptr_base :
        experimental::instance_provider<estd::internal::shared_ptr_inline_deleter_control_block<T, TDeleter> >
{
    typedef experimental::instance_provider<estd::internal::shared_ptr_inline_deleter_control_block<T, TDeleter> > base_type;

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
        experimental::instance_from_pointer_provider<estd::internal::shared_ptr_control_block<T> >
{
    typedef experimental::instance_from_pointer_provider<
        estd::internal::shared_ptr_control_block<T> > base_type;

protected:
    bool is_active() const { return this->value_ptr() != NULLPTR; }
    void deactivate() { this->value_ptr(NULLPTR); }

    shared_ptr_base(estd::internal::shared_ptr_control_block<T>* assign_from) :
        base_type(assign_from)
    {

    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    shared_ptr_base(shared_ptr_base&& move_from) :
        base_type(move_from.value_ptr())
    {

    }
#endif
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
    shared_ptr(experimental::instance_provider<estd::internal::shared_ptr_inline_deleter_control_block<T, TDeleter> >&
               copy_from) :
        base_type(copy_from.value().is_active ? &copy_from.value() : NULLPTR)
    {}

    template <class TControlBlock>
    shared_ptr(layer1::shared_ptr_base<TControlBlock>& copy_from) :
        base_type(copy_from.value().is_active ? &copy_from.value() : NULLPTR)
    {

    }

    /*
     * Doesn't help
    template <class Deleter, class TControlBlock>
    shared_ptr(layer1::shared_ptr<T, Deleter, TControlBlock>& copy_from) :
        base_type(copy_from.value().is_active ? &copy_from.value() : NULLPTR)
    {

    } */

    // value_ptr might be null here if copy_from isn't tracking anything
    shared_ptr(shared_ptr& copy_from) : base_type(copy_from.value_ptr()) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    shared_ptr(shared_ptr&& move_from) : base_type(std::move(move_from))
    {
        move_from.deactivate();
    }
#endif

    //template <class TDeleter>
    //shared_ptr& operator=(experimental::instance_provider<estd::internal::shared_ptr_inline_deleter_control_block<T, TDeleter> >&
    template <class TSharedPtr>
    shared_ptr& operator=(TSharedPtr&
                         copy_from)
    {
        this->reset();
        new (this) shared_ptr(copy_from);
        return *this;
    }

    /*
    template <class TControlBlock>
    shared_ptr& operator=(layer1::shared_ptr_base<TControlBlock>& assign_from)
    {
        this->reset();
        new (this) shared_ptr(assign_from);
        return *this;
    } */

    void swap(shared_ptr& r) noexcept
    {
        /*
        typename base_type::control_type* old = this->value_ptr();
        this->value_ptr(r.value_ptr());
        //r.deactivate();
        r.value_ptr(old); */

        r._value = estd::exchange(this->_value, r._value);
    }
};


}


template <class T, class TBase, class TBase2>
bool operator==(const experimental::shared_ptr2_base<T, TBase>& lhs,
                const experimental::shared_ptr2_base<T, TBase2>& rhs)
{
    return lhs.get() == rhs.get();
}


/*
 * Getting activated in a strange way so need to disable this
template <class T, class TBase, class TBase2>
bool operator<(const experimental::shared_ptr2_base<T, TBase>& lhs,
                const experimental::shared_ptr2_base<T, TBase2>& rhs)
{
    return lhs.get() < rhs.get();
} */


}
