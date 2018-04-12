#pragma once

#include <stdlib.h>
#include "iterator.h"

// TODO: utilize portions of std array here, if we can
// Note that std::array maps directly to our layer1 approach
// but we value add with layer2, layer3, etc.

namespace estd {

template<
    class T,
    std::size_t N,
    typename size_t = std::size_t
> struct array
{
private:
    T array[N];

public:
    typedef size_t size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    // https://stackoverflow.com/questions/3582608/how-to-correctly-implement-custom-iterators-and-const-iterators
    // I've read we're no longer supposed to use std::iterator.  I'm just gonna cobble together something usable then
    // for now
    class iterator
    {
        T* ptr;

        typedef int ptrdiff_t;

    public:
        iterator(T* ptr) : ptr(ptr) {}
        iterator(const iterator& copy_from) = default;

        iterator& operator++() {++ptr;return *this; }
        iterator& operator--() {--ptr;return *this; }
        iterator operator++(ptrdiff_t)
        { iterator temp(*this);++ptr;return temp;}

        iterator operator--(ptrdiff_t)
        { iterator temp(*this);--ptr;return temp;}

        bool operator==(const iterator compare) const
        {
            return ptr == compare.ptr;
        }

        bool operator!=(const iterator& compare) const
        {
            return compare.ptr != ptr;
        }

        bool operator>(const iterator& compare) const
        {
            return ptr > compare.ptr;
        }

        ptrdiff_t operator-(const iterator& compare) const
        {
            return ptr - compare.ptr;
        }

        T& operator*()              { return *ptr; }
        const T& operator*() const  { return *ptr; }
    };

    typedef const iterator const_iterator;

    iterator begin() { return iterator(array); }
    const_iterator begin() const { return iterator(array); }

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
    iterator end() { return iterator(&array[N]); }
    const_iterator end() const { return iterator(&array[N]); }

    CONSTEXPR size_type size() const { return N; }

    reference operator[](size_type pos)
    {
        return array[pos];
    }

    const_reference operator [](size_type pos) const
    {
        return array[pos];
    }
};

}
