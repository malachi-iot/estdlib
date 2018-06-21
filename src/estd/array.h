#pragma once

#include <stdlib.h>
#include "iterator.h"
#include "internal/runtime_array.h"
#include "internal/impl/allocated_array.h"
#include "allocators/fixed.h"

// TODO: utilize portions of std array here, if we can
// Note that std::array maps directly to our layer1 approach
// but we value add with layer2, layer3, etc.

namespace estd {

#define FEATURE_ESTD_LEGACY_ARRAY
#ifdef FEATURE_ESTD_LEGACY_ARRAY

namespace experimental {

struct array_traits
{
    typedef std::size_t size_type;
};


template<class T, class TArray, typename size_type = std::size_t>
struct array_base
{
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

protected:
    TArray m_array;

    array_base() {}

public:

    // FIX: perhaps array_base is overengineered and we can merely use
    // array
    CONSTEXPR size_type size() const { return sizeof(m_array) / sizeof(value_type); }

protected:

#ifdef FEATURE_CPP_INITIALIZER_LIST
    array_base(::std::initializer_list<value_type> init) //: m_array(init)
    {
        // init.size() and size() is not integral constant expression, darn - both
        // technically should be
        //static_assert(init.size() < size(), "Receiving array too small");

        T* a = m_array;

        for(auto it : init)
            *a++ = it;
    }
#endif

public:
    reference operator[](size_type pos)
    {
        return m_array[pos];
    }

    const_reference operator [](size_type pos) const
    {
        return m_array[pos];
    }


    // https://stackoverflow.com/questions/3582608/how-to-correctly-implement-custom-iterators-and-const-iterators
    // I've read we're no longer supposed to use std::iterator.  I'm just gonna cobble together something usable then
    // for now
    class iterator
    {
        T* ptr;

        typedef int ptrdiff_t;

    public:
        iterator(T* ptr) : ptr(ptr) {}
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
        iterator(const iterator& copy_from) = default;
#endif

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


        bool operator<(const iterator& compare) const
        {
            return ptr < compare.ptr;
        }

        ptrdiff_t operator-(const iterator& compare) const
        {
            return ptr - compare.ptr;
        }


        iterator operator-(const ptrdiff_t& movement) const
        {
            iterator temp(*this);

            temp.ptr -= movement;

            return temp;
        }


        iterator operator+(const ptrdiff_t& movement) const
        {
            iterator temp(*this);

            temp.ptr += movement;

            return temp;
        }


        T& operator*()              { return *ptr; }
        const T& operator*() const  { return *ptr; }
    };

    typedef const iterator const_iterator;

    iterator begin() { return iterator(m_array); }
    const_iterator begin() const { return iterator((T* const)m_array); }
};


}

template<
    class T,
    std::size_t N,
    typename size_t = std::size_t
> struct array : public experimental::array_base<T, T[N], size_t>
{
    typedef experimental::array_base<T, T[N]> base_t;

private:
    T* m_array() { return base_t::m_array; }

public:
    typedef size_t size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
    iterator end() { return iterator(&m_array()[N]); }
    const_iterator end() const { return iterator((T* const)&m_array()[N]); }

    CONSTEXPR size_type size() const { return N; }

    array() {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    array(::std::initializer_list<value_type> init) : base_t(init)
    {

    }
#endif
};

#ifdef FEATURE_CPP_VARIADIC
namespace experimental { namespace layer0 {

// fiddling with idea suggested here:
// https://stackoverflow.com/questions/19019252/create-n-element-constexpr-array-in-c11

template <class T, T... values>
class array_exp1
{
public:
    typedef T value_type;
};

template <class T, T value, T... values>
class array_exp1<T, value, values...> : array_exp1<T, values...>
{
    typedef array_exp1<T, values...> base_t;

public:
    typedef T value_type;
};


template <class T, T... values>
class array
{
public:
    typedef T value_type;
};

template <class T, T value, T... values>
class array<T, value, values...> : array<T, values...>
{

};



}}
#endif

namespace layer1 {
// TODO: alias estd::array into here
}

namespace layer2 {

// fixed size, pointer to buffer
template<
    class T,
    std::size_t N,
    typename size_t = std::size_t
> struct array : public experimental::array_base<T, T*, size_t>
{
    typedef experimental::array_base<T, T*> base_t;

private:
    T* m_array() { return base_t::m_array; }

public:
    array(T* const array) { base_t::m_array = array; }

    typedef size_t size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
    iterator end() { return iterator(&m_array()[N]); }
    const_iterator end() const { return iterator((T* const)&m_array()[N]); }

    CONSTEXPR size_type size() const { return N; }
};

}


namespace layer3 {

template<
    class T,
    typename size_t = std::size_t
> struct array : public experimental::array_base<T, T*, size_t>
{
    typedef experimental::array_base<T, T*, size_t> base_t;

private:
    size_t m_size;

public:
    array(T* const array, size_t size) : m_size(size)
    {
        base_t::m_array = array;
    }

    template <size_t N>
    array(T (&array) [N]) : m_size(N)
    {
        base_t::m_array = array;
    }

    typedef size_t size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;

    size_type size() const { return m_size; }

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
    iterator end() { return iterator(&base_t::m_array[size()]); }
    const_iterator end() const { return iterator((T* const)&base_t::m_array[size()]); }
};

}
#else

// FIX: Need Impl, not allocator itself for allocated_array
template <
        class T, size_t N,
        class TImpl = internal::impl::allocated_array<internal::single_fixedbuf_allocator<T, N, false> > >
class array : public internal::allocated_array<TImpl>
{
    typedef internal::allocated_array<TImpl> base_t;

public:

    typedef typename base_t::value_type value_type;

#ifdef FEATURE_CPP_INITIALIZER_LIST
    typedef ::std::initializer_list<value_type> initializer_list;

    array(initializer_list init)
    {
        std::copy(init.begin(), init.end(), base_t::lock());

        base_t::unlock();
    }
#endif

    array() {}
};

namespace layer2 {

template <
        class T, size_t N,
        class TImpl = internal::impl::allocated_array<internal::single_fixedbuf_allocator<T, N, false, T*> > >
class array : public internal::allocated_array<TImpl>
{
    typedef internal::allocated_array<TImpl> base_t;
public:

#ifdef FEATURE_CPP_INITIALIZER_LIST
#endif

    array(T* src) : base_t(src) {}
};

}

namespace layer3 {

template <
        class T,
        class TImpl = internal::impl::allocated_array<internal::single_fixedbuf_runtimesize_allocator<T, false> > >
class array : public internal::allocated_array<TImpl>
{
    typedef internal::allocated_array<TImpl> base_t;
    typedef typename base_t::impl_type impl_type;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::InitParam init_t;

public:

    template <size_t N>
    array(T (&array) [N]) : base_t(init_t(array, N))
    {
    }
};

}


#endif
}
