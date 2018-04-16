#pragma once

#include "estd/memory.h"

// reference allocator for inbuild mechanisms
class _allocator
{
public:
    // Would like to use this somehow to enable/disable auto-unlocking
    // for iterators
    static constexpr bool is_locking() { return false; }

    typedef estd::nothing_allocator::lock_counter lock_counter;

    typedef void* value_type;
    typedef void* pointer;
    typedef const void* const_void_pointer;
    typedef void* handle_type;
    typedef void* handle_offset_type;

    template <class T>
    class gcroot
    {
        handle_offset_type loc;

    public:
        gcroot(handle_offset_type loc) : loc(loc) {}

        T& lock() { return * (T*) loc; }

        void unlock() {}
    };

    static handle_type invalid() { return NULLPTR; }

    pointer lock(handle_type h) { return h; }

    void unlock(handle_type) { }

    void* allocate(size_t size)
    {
        return malloc(size);
    }

    void deallocate(void* p, size_t size)
    {
        free(p);
    }


    handle_type reallocate(handle_type h, size_t size)
    {
        return realloc(h, size);
    }


    static handle_offset_type offset(handle_type h, ptrdiff_t size)
    {
        return (uint8_t*)h + size;
    }
};

