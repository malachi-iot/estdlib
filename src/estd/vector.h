#pragma once

#include <stdlib.h>

namespace estd {

// FIX: this would be better suited in a more memory-specific area
// this represents a kind of pointer offset from an unlocked handle
template <class THandle, typename size_type = size_t>
struct handle_with_offset
{
    THandle handle;
    size_type offset;
};



template <class T, class Allocator>
class vector
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;

private:
    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::template gcroot<T> gcroot;

    allocator_type allocator;

    handle_type raw;

    // hopefully someday we can lean on allocator to tell us this
    size_type m_capacity;

    size_type m_size;

    T* lock()
    {
        return (T*) allocator.lock(raw);
    }

    void unlock() { allocator.unlock(raw); }

public:
    vector()
        : raw(allocator_type::invalid()), m_size(0) {}

    ~vector()
    {
        if(raw != allocator_type::invalid())
            allocator.deallocate(raw, m_capacity * sizeof(T));
    }


    class iterator
    {
    private:
        gcroot current;

    public:

    };

    size_type size() const { return m_size; }

    size_type capacity() const
    {
        if(raw == allocator_type::invalid()) return 0;

        return m_capacity;
    }

    void reserve( size_type new_cap )
    {
        raw = allocator.reallocate(raw, new_cap * sizeof(T));
        m_capacity = new_cap;
    }

    gcroot operator[](size_type pos)
    {
        return gcroot(allocator_type::offset(raw, pos * sizeof(T)));
    }

    void push_back(const T& value)
    {
        if(size() == capacity())
        {
            reserve(capacity() + 10);
        }

        T* v = lock();

        v[size()] = value;

        unlock();

        m_size++;
    }
};

}
