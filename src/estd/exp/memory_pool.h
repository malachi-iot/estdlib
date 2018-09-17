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
    typedef T value_type;

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



#ifndef NOTREADY
// totally proof of concepting. bad name, used for shared_ptr
template <class T, class TMemoryPool2>
struct memory_pool_item_traits<estd::layer1::shared_ptr<T, void>, TMemoryPool2 >
{
    //static constexpr int total_size()
    //{
    //    auto F = [](T*){};

    //    return sizeof(F);
    //}

    struct control_block : internal::shared_ptr_control_block2_base<T>
    {
        typedef internal::shared_ptr_control_block2_base<T> base_type;

        TMemoryPool2* pool;

        // should be a pointer to shared_ptr itself that comes from the construct operation
        // which very technically should also be the this-pointer but that feels a little
        // too hack-y right now
        void* pool_item;

        void Deleter() OVERRIDE
        {
            //pool.destroy(*this->shared);
            pool->destroy_internal(pool_item);
        }

        control_block(T* shared, bool is_active) : base_type(shared, is_active)
        {}
        /*
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
    typedef estd::layer1::shared_ptr<T, void, control_block> value_type;

    template <class TMemoryPool, class ...TArgs>
    static void construct(TMemoryPool& pool, value_type* value, TArgs...args)
    {
        // TODO: use allocator_traits
        //new (value) value_type([](TMemoryPool2& mp, T* d){});
        //new (value) value_type(deleter);
        value->_value.pool = &pool;
        value->_value.pool_item = value;
        new (value) value_type();
    }


    template <class TMemoryPool>
    static void destroy(TMemoryPool& pool, value_type& value)
    {
        value.~value_type();
    }
};
#endif

template <class T, std::ptrdiff_t N
          //class Traits = memory_pool_item_traits<T>
          >
class memory_pool_1
{
    typedef uint16_t size_type;
    //typedef Traits traits_type;
    typedef memory_pool_item_traits<T, memory_pool_1> traits_type;
    typedef typename traits_type::value_type value_type;

    struct item
    {
        size_type _next;

        size_type next() const { return _next; }

        estd::experimental::raw_instance_provider<value_type> value;
    };

    struct item_node_traits
    {
        typedef array<item, N> storage_type;
        //typedef typename aligned_storage<sizeof(item), alignof (item)>::type storage_type;
        // TODO: Phase this out into the external 'node allocator' and pass
        // in allocator to lock/unlock and friends
        storage_type storage;

        typedef item value_type;
        typedef item node_type;
        typedef size_type node_handle;
        typedef item& nv_ref_t;
        typedef nothing_allocator<item> node_allocator_type;

        static CONSTEXPR node_handle eol() { return numeric_limits::max<node_handle>(); }

        static item* adjust_from(typename traits_type::value_type * val)
        {
            item temp;

            int sz = (byte*)&temp.value - (byte*)&temp;

            item* _val = (item*)((byte*)val - sz);

            return _val;
        }

        node_type& lock(node_handle h)
        {
            return storage[h];
        }

        void unlock(node_handle) {}

        // node allocation is specifically taking the 'value' portion and allocating
        // + associating just the node portion with it.  for intrusive lists, this
        // is largely a noop - we merely resolve where the pointer lives and report
        // its already-allocated handle from the storage area
        node_handle allocate(nv_ref_t n)
        {
            item* data = storage.data();
            item* _n = &n;
            node_handle h = _n - data;
            return h;
        }

        void deallocate(node_handle) {}

        node_handle next(node_type& n)
        {
            return n.next();
        }

        void next(node_type& to_attach_to, node_handle& n2)
        {
            to_attach_to._next = n2;
        }
    };

    // TODO: we can simplify & optimize this and have the traits live completely inside the
    // list.  again clumsy because traits aren't typically thought of as stateful
    //item_node_traits traits;

    typedef internal::forward_list<item, item, nothing_allocator<item>, item_node_traits> list_type;
    list_type free;
    //intrusive_forward_list<item> free;

public:
    memory_pool_1()
    {
        item_node_traits& t = free.get_traits();

        // prime the intrusive list
        free.push_front(t.storage[0]);

        for(int i = 0; i < N - 1; i++)
            t.storage[i]._next = i + 1;

        t.storage[N - 1]._next = item_node_traits::eol();
    }

    size_type count_free() const
    {
        return distance(free.begin(), free.end());
    }

    ///
    /// \brief allocates but does not construct the item
    /// \return
    ///
    value_type* allocate()
    {
        if(free.empty()) return NULLPTR;

        item& to_allocate = free.front();

        free.pop_front();

        return &to_allocate.value.value();
    }

    void deallocate(value_type* to_free)
    {
        free.push_front(*item_node_traits::adjust_from(to_free));
    }

#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    value_type& construct(TArgs&&...args)
    {
        value_type* value = allocate();

        traits_type::construct(*this, value, std::forward<TArgs>(args)...);

        return *value;
    }
#endif


    void destroy(value_type& value)
    {
        traits_type::destroy(*this, value);
        deallocate(&value);
    }


    // only for use from shared_ptr control structure.  value is still
    // value_type, but it's hard for shared_ptr control structure to
    // know that exact type at that time
    void destroy_internal(void* value)
    {
        destroy(*(value_type*)value);
    }
};

}}
