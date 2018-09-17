/**
 * @file
 */
#pragma once

#include "../array.h"
#include "../type_traits.h"
#include "../cstdint.h"
#include "../forward_list.h"
#include "../algorithm.h"

// memory pools continually turn out to be kinda tricky, so numbering my attempts
namespace estd { namespace experimental {

template <class T, std::ptrdiff_t N, std::ptrdiff_t align>
struct typed_aligned_storage
{

};

template <class T, std::ptrdiff_t N>
class memory_pool_1
{
    typedef uint16_t size_type;

    struct item
    {
        size_type _next;

        size_type next() const { return _next; }

        estd::experimental::raw_instance_provider<T> value;
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

        static CONSTEXPR node_handle eol() { return -1; }

        static item* adjust_from(T* val)
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
    T* allocate()
    {
        if(free.empty()) return NULLPTR;

        item& to_allocate = free.front();

        free.pop_front();

        return &to_allocate.value.value();
    }

    void deallocate(T* to_free)
    {
        free.push_front(*item_node_traits::adjust_from(to_free));
    }

#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    T& construct(TArgs&&...args)
    {
        T* value = allocate();

        // TODO: use allocator_traits
        new (value) T(std::forward<TArgs>(args)...);

        return *value;
    }


    void destroy(T& value)
    {
        value.~T();
        deallocate(&value);
    }
#endif
};

}}
