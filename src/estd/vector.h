#pragma once

#include <stdlib.h>
#include "memory.h"

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
class vector : public experimental::dynamic_array<T, Allocator>
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;

private:
    typedef experimental::dynamic_array<T, Allocator> base_t;
    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::template gcroot<T> gcroot;

public:
    class iterator
    {
    private:
        gcroot current;

    public:

    };

    gcroot operator[](size_type pos)
    {
        return gcroot(allocator_type::offset(base_t::handle, pos * sizeof(T)));
    }
};

}
