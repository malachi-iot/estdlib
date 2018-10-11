/**
 * @file
 */
#pragma once

#include "../array.h"
#include "../memory.h"
#include "../type_traits.h"
#include "../cstdint.h"
#include "../forward_list.h"
#include "../algorithm.h"
#include "../limits.h"

// memory pools continually turn out to be kinda tricky, so numbering my attempts
namespace estd { namespace experimental {

template <class T, std::ptrdiff_t N, std::ptrdiff_t align>
struct typed_aligned_storage
{

};


// Experimental - just here to try to suppose shared_ptr specialization, but that
// isn't going so well
template <class T, class TMemoryPool2>
struct memory_pool_item_traits
{
    typedef T tracked_value_type;
    typedef TMemoryPool2 pool_type;

    template <class TMemoryPool, class ...TArgs>
    static void construct(TMemoryPool& pool, T* value, TArgs...args)
    {
        // TODO: use allocator_traits
        new (value) T(std::forward<TArgs>(args)...);
    }

    template <class TMemoryPool>
    static void destroy(TMemoryPool& pool, T& value)
    {
        value.~T();
    }
};



//#define FEATURE_ESTD_EXP_AUTOCONSTRUCT

// totally proof of concepting. bad name, used for shared_ptr
template <class T, class TMemoryPool>
struct memory_pool_item_traits<estd::layer1::shared_ptr<T, void>, TMemoryPool >
{
    typedef TMemoryPool pool_type;
    //static constexpr int total_size()
    //{
    //    auto F = [](T*){};

    //    return sizeof(F);
    //}

    /**
     * @brief Specialized shared_ptr control block for memory pool interaction
     */
    struct control_block : internal::shared_ptr_control_block<T>
    {
        typedef internal::shared_ptr_control_block<T> base_type;

        TMemoryPool* pool;

        // should be a pointer to shared_ptr itself that comes from the construct operation
        // which very technically should also be the this-pointer but that feels a little
        // too hack-y right now
        void* pool_item;

        ///
        /// \brief interacts directly with memory pool to free memory from there
        ///
        /// runs this object's destructor also (destroy)
        ///
        void Deleter() OVERRIDE
        {
            pool->destroy_internal(pool_item);
        }

        control_block(T* shared, bool is_active) : base_type(shared, is_active)
        {}

        /* NOTE: this would be better, but currently very complicated to integrate into
         *  architecture
        control_block(TMemoryPool2& pool, void* pool_item) :
            pool(pool, pool_item) {} */
    };


    //static void deleter(TMemoryPool2& mp, T* d)
    //{
    //    mp.destroy(*d);
    //}

    // can't do this, according to https://stackoverflow.com/questions/4846540/c11-lambda-in-decltype
    //typedef estd::layer1::shared_ptr<T, std::decltype([](T*){})> value_type;
    //typedef estd::layer1::shared_ptr<T, void> value_type;
    typedef estd::layer1::shared_ptr<T, void, control_block> tracked_value_type;

    template <class ...TArgs>
    static void construct(TMemoryPool& pool, tracked_value_type* value, TArgs&&...args)
    {
        // TODO: use allocator_traits
        //new (value) value_type([](TMemoryPool2& mp, T* d){});
        //new (value) value_type(deleter);
        value->_value.pool = &pool;
        value->_value.pool_item = value;
        new (value) tracked_value_type();

#ifdef FEATURE_ESTD_EXP_AUTOCONSTRUCT
        value->construct(std::forward<TArgs>(args)...);
#endif
    }


    static void destroy(TMemoryPool& pool, tracked_value_type& value)
    {
        value.~tracked_value_type();
    }
};

// making a base here so we can reuse componentry between pointer-tracked linked list
// and handle-based linked list varieties
template <class T, std::size_t N>
class memory_pool_base
{
protected:
    typedef typename internal::deduce_fixed_size_t<N>::size_type size_type;

    ///
    /// \brief Internal pool entry
    ///
    /// aligned along 'T' boundaries so that any pointers etc. in T don't cause issues
    ///
    template <class TTrackedValue, class THandle>
    struct
#ifdef FEATURE_CPP_ALIGN
            //alignas (alignof (value_type))
#else
//#error Memory pool requires memory alignment
#endif
            item
    {
        estd::experimental::raw_instance_provider<TTrackedValue> value;

        // put this here to ease pressure on alignment issues.  Presumably this can land on
        // any alignment because it's not a pointer type, but because it may be a multibyte
        // integer it might still need to be aligned?  we'll find out
        THandle _next;
    };


    // TNextHandle is useful to brute force back to a void* for item._next in here
    template <class TBase, class TNextHandle = typename TBase::handle_type>
    struct _item_node_traits : TBase
    {
        typedef TBase base_type;
        typedef typename base_type::tracked_value_type tracked_value_type;
        typedef typename base_type::handle_type node_handle;
        typedef TNextHandle next_node_handle;
        // FIX: Pretty sure this line is goofing us up - it's basically fine, but
        // we're feeding item<TTrackedValue, void*> around and I think they don't mesh
        typedef item<tracked_value_type, next_node_handle> node_type;
        typedef node_type& nv_ref_t;
        typedef nothing_allocator<node_type> node_allocator_type;

        _item_node_traits() {}

        // Should only use this when in byref=true mode
        template <class TParam>
        _item_node_traits(TParam& p) : base_type(p) {}

        static node_type* adjust_from(tracked_value_type * val)
        {
            node_type temp;

            int sz = (byte*)&temp.value - (byte*)&temp;

            node_type* _val = (node_type*)((byte*)val - sz);

            return _val;
        }

        void deallocate(node_handle) {}

        static node_handle next(node_type& n)
        {
            return reinterpret_cast<node_handle>(n._next);
        }

        static void next(node_type& to_attach_to, node_handle& n2)
        {
            to_attach_to._next = n2;
        }
    };
};


template <class T, std::size_t N, class TTraits>
class memory_pool_ll_base : public memory_pool_base<T, N>
{
    typedef memory_pool_base<T, N> base_type;

protected:
    typedef TTraits traits_type;
    //typedef memory_pool_item_traits<T, memory_pool_handle_base> traits_type;
    typedef typename traits_type::tracked_value_type tracked_value_type;

    typedef typename base_type::template item<tracked_value_type,
            void*> item;

    // no byref here, because we can use a pretty normal intrusive forward list
    // on this one (no translation from handles, we are using pointers for next)
    struct item_storage_exp
    {
        typedef item value_type;
        typedef item* handle_type;

        // disambiguate value_type of consumer interest vs. value_type that intrusive list
        // needs to operate on (which will be our 'item' class)
        typedef typename traits_type::tracked_value_type tracked_value_type;
        typedef array<item, N> storage_type;

        static CONSTEXPR handle_type eol() { return NULLPTR; }

        //typedef typename aligned_storage<sizeof(item), alignof (item)>::type storage_type;
        // TODO: Phase this out into the external 'node allocator' and pass
        // in allocator to lock/unlock and friends
        storage_type storage;

        value_type& lock(handle_type h)
        {
            return *h;
        }

        void unlock(handle_type) {}

        // node allocation is specifically taking the 'value' portion and allocating
        // + associating just the node portion with it.  for intrusive lists, this
        // is largely a noop - we merely resolve where the pointer lives and report
        // its already-allocated handle from the storage area
        handle_type allocate(value_type& n)
        {
            return &n;
        }
    };

    typedef typename base_type::template _item_node_traits<
            item_storage_exp, void* > item_node_traits;
};

// base class specifically for handle-based memory pool.  sets up all requisite typedefs
template <class T, std::size_t N, class TTraits>
class memory_pool_handle_base : public memory_pool_base<T, N>
{
    typedef memory_pool_base<T, N> base_type;

public:
    typedef typename base_type::size_type size_type;

    //typedef Traits traits_type;
    typedef TTraits traits_type;
    //typedef memory_pool_item_traits<T, memory_pool_handle_base> traits_type;
    typedef typename traits_type::tracked_value_type tracked_value_type;

#ifdef UNIT_TESTING
public:
#else
    protected:
#endif
    struct item_node_traits_base
    {
        typedef size_type handle_type;
        // disambiguate value_type of consumer interest vs. value_type that intrusive list
        // needs to operate on (which will be our 'item' class)
        typedef typename traits_type::tracked_value_type tracked_value_type;

        static CONSTEXPR handle_type eol() { return numeric_limits::max<handle_type>(); }
    };

    typedef typename base_type::template item<tracked_value_type,
            typename item_node_traits_base::handle_type> item;

    // going to be challenging, but attempt to decouple node storage/allocator
    // from node traits itself
    // doing this byref trickery so that we can make external linked lists which use
    // this memory pool's internal storage
    template <bool byref>
    struct item_storage_exp : item_node_traits_base
    {
        typedef item_node_traits_base base_type;
        typedef typename base_type::handle_type handle_type;
        typedef item value_type;
        typedef array<item, N> _storage_type;

        typedef typename estd::conditional<byref, _storage_type&, _storage_type>::type storage_type;

        //typedef typename aligned_storage<sizeof(item), alignof (item)>::type storage_type;
        // TODO: Phase this out into the external 'node allocator' and pass
        // in allocator to lock/unlock and friends
        storage_type storage;

        item_storage_exp() {}

        // Should only use this when in byref=true mode
        item_storage_exp(storage_type& copy_by_ref) : storage(copy_by_ref) {}

        value_type& lock(handle_type h)
        {
            return storage[h];
        }

        void unlock(handle_type) {}

        // node allocation is specifically taking the 'value' portion and allocating
        // + associating just the node portion with it.  for intrusive lists, this
        // is largely a noop - we merely resolve where the pointer lives and report
        // its already-allocated handle from the storage area
        handle_type allocate(value_type& n)
        {
            value_type* data = this->storage.data();
            value_type* _n = &n;
            handle_type h = _n - data;
            return h;
        }
    };


    typedef typename base_type::template _item_node_traits<
            item_storage_exp<false> > item_node_traits;

    typedef typename base_type::template _item_node_traits<
            item_storage_exp<true> > item_ext_node_traits;

    typedef internal::forward_list<item, item, nothing_allocator<item>, item_ext_node_traits > ext_list_type;

};

template <class TBase>
///
/// \brief Memory pool utilizing a handle-based intrusive linked list
///
/// Starts out with N slots, each slot comprised of 'item' structure which in turn
/// has memory space for a full T.  It's managed so that it's properly unconstructed
///
class memory_pool_internal :
        //public memory_pool_handle_base<T, N, memory_pool_item_traits<T, memory_pool_1<T, N> > >
        public TBase
{
    //typedef memory_pool_handle_base<T, N, memory_pool_item_traits<T, memory_pool_1<T, N> > > base_type;
    typedef TBase base_type;

public:
    typedef typename base_type::size_type size_type;

    //typedef Traits traits_type;
    typedef typename base_type::traits_type traits_type;
    typedef typename traits_type::tracked_value_type value_type;

public:
    //typedef typename base_type::item_node_traits_base item_node_traits_base;
    typedef typename base_type::item_node_traits item_node_traits;
    typedef typename base_type::item item;

    // TODO: we can simplify & optimize this and have the traits live completely inside the
    // list.  again clumsy because traits aren't typically thought of as stateful
    //item_node_traits traits;

    // a bit clusmy, item_node_traits instance lives inside forward list, and that also is where
    // our memory pool is living (item_storage_exp).  Works efficiently, but convoluted.  This
    // is why we want to decouple it from traits if we can
    typedef internal::forward_list<item, item, nothing_allocator<item>, item_node_traits> list_type;

protected:
    list_type free;
    //intrusive_forward_list<item> free;

#ifdef UNIT_TESTING
public:
#endif

    item_node_traits& node_traits()
    {
        return free.get_traits();
    }

    // internal call to easily access underlying pool storage
    item* storage()
    {
        return node_traits().storage.data();
    }

public:
    size_type count_free() const
    {
        return distance(free.begin(), free.end());
    }

    // low-level allocation, returns also easy access to metadata.  Presumes
    // that there is an item available (that free.empty() == false)
    item& allocate_item()
    {
        item& to_allocate = free.front();

        free.pop_front();

        return to_allocate;
    }

    ///
    /// \brief allocates but does not construct the item
    /// \return
    ///
    value_type* allocate()
    {
        if(free.empty()) return NULLPTR;

        item& to_allocate = allocate_item();

        return &to_allocate.value.value();
    }


    void deallocate_item(item& to_free)
    {
        free.push_front(to_free);
    }

    ///
    /// \brief frees pool slot in which 'to_free' resides
    /// \param to_free
    ///
    void deallocate(value_type* to_free)
    {
#ifdef DEBUG
        // TODO: do sanity check to make sure to_free resides in proper pointer space
#endif
        deallocate_item(*item_node_traits::adjust_from(to_free));
    }

#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    ///
    /// \brief allocate and construct
    /// \param args
    /// \return
    ///
    value_type& construct(TArgs&&...args)
    {
        value_type* value = allocate();
        // FIX: Repair this awful forward cast.  We have to do this because
        // traits_type needs to interact directly with the proper memory pool type
        // (that we are sourced from) but architecture's weakness makes that tricky
        auto _this = (typename traits_type::pool_type*)this;

        traits_type::construct(*_this, value, std::forward<TArgs>(args)...);

        return *value;
    }
#endif


    ///
    /// \brief execute destructor and deallocate
    /// \param value
    ///
    void destroy(value_type& value)
    {
        // FIX: Repair this awful forward cast.  We have to do this because
        // traits_type needs to interact directly with the proper memory pool type
        // (that we are sourced from) but architecture's weakness makes that tricky
        auto _this = (typename traits_type::pool_type*)this;

        traits_type::destroy(*_this, value);
        deallocate(&value);
    }


    // only for use from shared_ptr control structure.  value is still
    // value_type, but it's hard for shared_ptr control structure to
    // know that exact type at that time
    void destroy_internal(void* value)
    {
        destroy(*reinterpret_cast<value_type*>(value));
    }

    // Bad name.  Acquire item from pool by handle directly
    item& lock(typename item_node_traits::node_handle h)
    {
        return free.get_traits().lock(h);
        //return *(storage() + h);
    }
};

template <class T, std::ptrdiff_t N>
struct memory_pool_1 : public
        memory_pool_internal<
        memory_pool_handle_base<T, N, memory_pool_item_traits<T, memory_pool_1<T, N> > > >
{
    typedef memory_pool_internal<
            memory_pool_handle_base<T, N, memory_pool_item_traits<T, memory_pool_1<T, N> > > > base_type;

    typedef typename base_type::item_node_traits_base item_node_traits_base;
    typedef typename base_type::item_node_traits item_node_traits;

    memory_pool_1()
    {
        item_node_traits& t = base_type::node_traits();

        // prime the intrusive list
        base_type::free.push_front(t.storage[0]);

        for(int i = 0; i < N - 1; i++)
            t.storage[i]._next = i + 1;

        t.storage[N - 1]._next = item_node_traits_base::eol();
    }
};


template <class T, std::ptrdiff_t N>
struct memory_pool_ll : public
                       memory_pool_internal<
                               memory_pool_ll_base<T, N, memory_pool_item_traits<T, memory_pool_ll<T, N> > > >
{
    typedef memory_pool_internal<
            memory_pool_ll_base<T, N, memory_pool_item_traits<T, memory_pool_ll<T, N> > > > base_type;

    typedef typename base_type::item_node_traits item_node_traits;
    typedef typename base_type::item item;

    memory_pool_ll()
    {
        item_node_traits& t = base_type::node_traits();
        item& first = t.storage[0];

        // prime the intrusive list
        // FIX: for some reason this line is unhappy
        base_type::free.push_front(first);

        for(int i = 0; i < N - 1; i++)
            t.storage[i]._next = &t.storage[i + 1];

        t.storage[N - 1]._next = item_node_traits::eol();
    }
};

#ifdef FEATURE_CPP_VARIADIC
ESTD_FN_HAS_TYPEDEF_EXP(element_type);
ESTD_FN_HAS_TYPEDEF_EXP(value_type);

template <class TMemoryPool, class ...TArgs>
typename TMemoryPool::value_type& make_shared(TMemoryPool& pool, TArgs&&...args)
{
    typedef typename TMemoryPool::value_type value_type;

#ifdef FEATURE_CPP_STATIC_ASSERT
    static_assert(has_value_type_typedef<TMemoryPool>::value, "Expecting memory pool with value_type");
    static_assert(has_element_type_typedef<value_type>::value, "Expecting shared_ptr with element_type");
#endif

    value_type& value = pool.construct();

    value.construct(std::forward<TArgs>(args)...);

    return value;
}
#endif


}}
