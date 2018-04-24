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



template <class T, template <class> class Allocator = std::allocator>
class vector : public experimental::dynamic_array<T, Allocator>
{
public:
    typedef T value_type;
    typedef Allocator<T> allocator_type;
    typedef std::size_t size_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

private:
    typedef experimental::dynamic_array<T, Allocator> base_t;
    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

    typedef allocator_traits<allocator_type> allocator_traits_t;

public:
    // NOTE: accessor may very well become interchangeable with iterator
    // used to be the most granular access to an array/vector element without
    // having to lock it
    class accessor : public handle_with_offset
    {
        operator T()
        {
            // copies it - beware, some T we don't want to copy!
            T retval = handle_with_offset::lock();

            handle_with_offset::unlock();

            return retval;
        }

        accessor& operator=(const T& assign_from)
        {
            T& value = handle_with_offset::lock();

            value = assign_from;

            handle_with_offset::unlock();

            return *this;
        }
    };

    class iterator
    {
    private:
        handle_with_offset current;

    public:
        iterator(const handle_with_offset& current) : current(current) {}
    };

    typedef const iterator const_iterator;

    handle_with_offset operator[](size_type pos)
    {
        return handle_with_offset(allocator_type::offset(base_t::handle, pos));
    }

    // TODO: consolidate with dynamic_array
    iterator begin()
    {
        handle_with_offset offset = allocator_type::offset(base_t::handle, 0);

        return iterator(offset);
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    iterator insert(const_iterator pos, const_reference value)
    {
        T* a = base_t::lock();

        T& pos_item = pos.lock();

        pos.unlock();

        base_t::unlock();
    }
#else
    iterator insert(iterator pos, const T& value)
    {

    }
#endif
};


}
