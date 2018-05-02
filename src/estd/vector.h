#pragma once

#include <stdlib.h>
#include "memory.h"
#include "internal/dynamic_array.h"
#include "allocators/fixed.h"

namespace estd {

template <class T, class Allocator = std::allocator<T > >
class vector : public internal::dynamic_array<Allocator>
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

private:
    typedef internal::dynamic_array<Allocator> base_t;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::handle_with_offset handle_with_offset;

    typedef allocator_traits<allocator_type> allocator_traits_t;

public:
    typedef typename base_t::size_type size_type;
    typedef typename base_t::accessor accessor;


    class iterator
    {
    private:
        accessor current;

    public:
        // All-or-nothing, though not supposed to be that way till C++17 but is sometimes
        // before that (http://en.cppreference.com/w/cpp/iterator/iterator_traits)
        typedef T value_type;
        typedef int difference_type;
        typedef T* pointer;
        typedef T& reference;
        typedef ::std::forward_iterator_tag iterator_category;

        iterator(const accessor& current) : current(current) {}

        iterator(const iterator& copy_from) : current(copy_from.current) {}

        // prefix version
        iterator& operator++()
        {
            current.h_exp().increment();
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
            return current.h_exp() == compare_to.current.h_exp();
        }

        bool operator!=(const iterator& compare_to) const
        {
            return !(operator ==)(compare_to);
            //return current != compare_to.current;
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

    allocator_type& get_allocator()
    {
        return base_t::get_allocator();
    }

    typename base_t::accessor operator[](size_type pos)
    {
        return base_t::operator [](pos);
    }

    // TODO: consolidate with dynamic_array
    iterator begin()
    {
        //handle_with_offset offset = get_allocator().offset(base_t::handle, 0);
        handle_with_offset offset = base_t::helper.offset(0);
        accessor a(get_allocator(), offset);

        return iterator(a);
    }

    iterator end()
    {
        //handle_with_offset offset = get_allocator().offset(base_t::handle, base_t::size());
        handle_with_offset offset = base_t::helper.offset(base_t::size());
        accessor a(get_allocator(), offset);

        return iterator(a);
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


namespace layer1 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N > >
{
};

};


namespace layer2 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, false, T* > >
{
    typedef estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, false, T* > > base_t;

public:
    vector(T* underlying_buffer) : base_t(underlying_buffer) {}
};

}

}
