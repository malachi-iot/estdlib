/*
 * @file
 */
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


#define FEATURE_ESTD_ARRAY_PROVIDER


template<class T, class TArray, typename TSize = std::size_t,
         class TProvider = instance_provider<TArray> >
struct array_base
#ifdef FEATURE_ESTD_ARRAY_PROVIDER
        : TProvider
#endif
{
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef TSize size_type;

protected:
#ifdef FEATURE_ESTD_ARRAY_PROVIDER
    typedef TProvider base_type;
#else
    TArray m_array;
#endif

    array_base() {}

    array_base(const TArray& array) :
#ifdef FEATURE_ESTD_ARRAY_PROVIDER
        base_type(array)
#else
        m_array(array)
#endif
    {

    }

public:
#ifdef FEATURE_ESTD_ARRAY_PROVIDER
    T* data() { return base_type::value(); }

    const T* data() const { return base_type::value(); }
#else
    // Normally we deter attempts to directly acquire data, but arrays really do
    // represent hard data (even later when we utilize allocated_array as its
    // base class) so go ahead and expose this
    T* data() { return m_array; }

    const T* data() const { return m_array; }
#endif

    // FIX: perhaps array_base is overengineered and we can merely use
    // array
    //CONSTEXPR size_type size() const { return sizeof(m_array) / sizeof(value_type); }

protected:

#ifdef FEATURE_CPP_INITIALIZER_LIST
    array_base(::std::initializer_list<value_type> init) //: m_array(init)
    {
        // init.size() and size() is not integral constant expression, darn - both
        // technically should be
        //static_assert(init.size() < size(), "Receiving array too small");

        T* a = data();

        for(auto it : init)
            *a++ = it;
    }
#endif

public:
    reference operator[](size_type pos)
    {
        return data()[pos];
    }

    const_reference operator [](size_type pos) const
    {
        return data()[pos];
    }


#ifdef FEATURE_ESTD_LEGACY_ARRAY_ITERATOR
    // https://stackoverflow.com/questions/3582608/how-to-correctly-implement-custom-iterators-and-const-iterators
    // I've read we're no longer supposed to use std::iterator.  I'm just gonna cobble together something usable then
    // for now
    class iterator
    {
        T* ptr;

        typedef int ptrdiff_t;

    public:
        // As per https://en.cppreference.com/w/cpp/iterator/iterator_traits it's important for
        // an iterator to define these.  GCC (but not clang) seems to demand a separate iterator_traits
        // in C++11 always.  Putting these typedefs in is an experiment to see
        typedef int difference_type;
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;
        typedef std::random_access_iterator_tag iterator_tag;

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

#else
    typedef T* iterator;
#endif

    typedef const iterator const_iterator;

    reference front() { return data()[0]; }
    const_reference front() const { return  data()[0]; }

#ifdef FEATURE_ESTD_LEGACY_ARRAY_ITERATOR
    iterator begin() { return iterator(m_array); }
    const_iterator begin() const { return iterator((T* const)m_array); }
#else
    T* begin() { return data(); }
    const T* begin() const { return data(); }
#endif
};


}

template<
    class T,
    std::size_t N,
    typename TSize = typename internal::deduce_fixed_size_t<N>::size_type
> struct array : public experimental::array_base<T, T[N], size_t>
{
    typedef experimental::array_base<T, T[N], size_t> base_t;

public:
    typedef TSize size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
#ifdef FEATURE_ESTD_LEGACY_ARRAY_ITERATOR
    iterator end() { return iterator(base_t::m_array + N); }
    const_iterator end() const { return iterator(base_t::m_array + N); }
#else
    T* end() { return base_t::data() + N; }
    const T* end() const { return base_t::data() + N; }
#endif

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

// since this generates annoying warnings under some circumstances (esp32),
// AND it's experimental, only enabling during unit testing
#ifdef UNIT_TESTING
// FIX: So far is not viable under gcc, but not sure why.  Merely rejects
// anything coming into 'value'.  Maybe because 'T* const'?
template <class T, T* const value, size_t N>
struct array_exp2 :
        array_base<T, T* const,
            size_t,
            //typename estd::internal::deduce_fixed_size_t<N>::type,
            global_pointer_provider<T, value> >
{
    typedef array_base<T, T* const,
        size_t,
        //typename estd::internal::deduce_fixed_size_t<N>::type,
        global_pointer_provider<T, value> > base_type;
    typedef typename base_type::size_type size_type;
    //typedef typename base_type::const_iterator const_iterator;

    typedef const T* const const_iterator;
    typedef T* iterator;

    // just because pointer casting is different, it creates some warnings, so brute
    // force code again to reduce those warnings
    // esp32 generates its own set of compiler warnings from this though
    iterator begin() { return base_type::data(); }
    const_iterator begin() const { return base_type::data(); }

    iterator end() { return base_type::data() + N; }
    const_iterator end() const { return base_type::data() + N; }
};
#endif

/*
 * Won't work because 'array' would need to be a constexpr, which is
 * invalid as a function parameter modifier
template <class T, size_t N>
constexpr auto make_array(T (&array)[N]) -> array_exp2<T, array, N>
{
    return array_exp2<T, array, N>(array);
} */

/*
template <class T, class TArray, size_t N = sizeof(TArray) / sizeof(T)>
constexpr array_exp2<T, TArray, N> make_array()
{
}
*/
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
    typename size_t = typename estd::internal::deduce_fixed_size_t<N>::size_type
> struct array : public experimental::array_base<T, T*, size_t>
{
    typedef experimental::array_base<T, T*, size_t> base_t;

private:
    //T* m_array() { return base_t::m_array; }

public:
    array(T* const array) : base_t(array) {}
    //{ base_t::m_array = array; }

    typedef size_t size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
    iterator end() { return iterator(base_t::data() + N); }
    const_iterator end() const { return iterator(base_t::data() + N); }

    CONSTEXPR size_type size() const { return N; }
};


template <class T, ptrdiff_t N>
estd::layer2::array<T, N> make_array(T (&a)[N])
{
    return estd::layer2::array<T, N>(a);
}


}


namespace layer3 {

template<
    class T,
    typename size_t = std::size_t
> struct array : public experimental::array_base<T, T*, size_t>
{
    typedef experimental::array_base<T, T*, size_t> base_t;

protected:
    size_t m_size;

public:
    array(T* const array, size_t size) :
        base_t(array),
        m_size(size)
    {
    }

    template <size_t N>
    array(T (&array) [N]) :
        base_t(array),
        m_size(N)
    {
    }

    typedef size_t size_type;
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename base_t::iterator iterator;
    typedef const iterator const_iterator;

    size_type size() const { return m_size; }

    reference back() { return base_t::data()[size() - 1]; }
    const_reference back() const { return base_t::data()[size() - 1]; }

    // NOTE: I don't like how C++ std implies 'past the end' on an array here,
    // pretty sure though we can fake it out with a NULL later on
    iterator end() { return iterator(&base_t::data()[size()]); }
    const_iterator end() const { return iterator((T* const)&base_t::data()[size()]); }
};

template <class T, ptrdiff_t N,
          class size_t = typename internal::deduce_fixed_size_t<N>::size_type>
///
/// \brief returns a layer3 array with a deduced size_t based on input array size
/// \return
///
estd::layer3::array<T, size_t> make_array(T (&a)[N])
{
    return estd::layer3::array<T, size_t>(a);
}


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
