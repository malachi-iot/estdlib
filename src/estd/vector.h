#pragma once

#include <stdlib.h>
#include "memory.h"
#include "internal/dynamic_array.h"

namespace estd {

// FIX: this would be better suited in a more memory-specific area
// this represents a kind of pointer offset from an unlocked handle
template <class THandle, typename size_type = size_t>
struct handle_with_offset
{
    THandle handle;
    size_type offset;
};



template <class T, class Allocator = std::allocator<T>>
class vector : public internal::dynamic_array<Allocator>
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

private:
    typedef internal::dynamic_array<Allocator> base_t;
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

        iterator(const iterator& copy_from) : current(copy_from.current) {}

        // prefix version
        iterator& operator++()
        {
            ++current;
            return *this;
        }

        // postfix version
        iterator operator++(int)
        {
            iterator temp(*this);
            operator++();
            return temp;
        }

        bool operator==(const iterator& compare_to) const
        {
            return current == compare_to.current;
        }

        bool operator!=(const iterator& compare_to) const
        {
            return current != compare_to.current;
        }

        value_type& lock() { return current.lock(); }
        void unlock() { current.unlock(); }

        T* operator*()
        {
            // TODO: consolidate with InputIterator behavior from iterators/list.h
            return &lock();
        }
    };

    typedef const iterator const_iterator;

    allocator_type get_allocator() const
    {
        return base_t::get_allocator();
    }

    handle_with_offset operator[](size_type pos)
    {
        return base_t::offset(pos);
    }

    // TODO: consolidate with dynamic_array
    iterator begin()
    {
        //handle_with_offset offset = get_allocator().offset(base_t::handle, 0);
        handle_with_offset offset = base_t::helper.offset(0);

        return iterator(offset);
    }

    iterator end()
    {
        //handle_with_offset offset = get_allocator().offset(base_t::handle, base_t::size());
        handle_with_offset offset = base_t::helper.offset(base_t::size());

        return iterator(offset);
    }

    const_iterator end() const
    {
        //handle_with_offset offset = get_allocator().offset(base_t::handle, base_t::size());
        handle_with_offset offset = base_t::helper.offset(base_t::size());

        return iterator(offset);
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    // NOTE: because pos requires a non-const lock, we can't do traditional
    // const_iterator here
    iterator insert(iterator pos, const_reference value)
    {
        T* a = base_t::lock();

        T& pos_item = pos.lock();

        // all very raw array dependent
        base_t::raw_insert(a, &pos_item, &value);

        pos.unlock();

        base_t::unlock();

        return pos;
    }
#else
    iterator insert(iterator pos, const T& value)
    {

    }
#endif
};


}
