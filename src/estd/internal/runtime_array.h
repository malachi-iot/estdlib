#pragma once

#include "platform.h"
#include "../traits/allocator_traits.h"
#include "../internal/accessor.h"
#include "../algorithm.h"
#include "../initializer_list.h"
#include "../iterator.h"

namespace estd { namespace internal {

// experimental and not good
// maxStringLength needs a clue to not inspect a type for length so that
// we don't get 'not implemented' problems with SFINAE
struct no_max_string_length_tag {};

// base class for any 'allocated' array, even ones using a fixed allocator
// which pushes the boundaries of what allocated even really means
// importantly, this allocated_array doesn't provide for growing/shrinking the array
template <class TImpl>
class allocated_array : no_max_string_length_tag
{
public:
    typedef TImpl impl_type;
    typedef typename impl_type::allocator_type allocator_type;
    typedef typename impl_type::allocator_traits allocator_traits;
    typedef typename allocator_traits::handle_type handle_type;
    //typedef typename allocator_traits::handle_with_size handle_with_size;
    typedef typename allocator_traits::pointer pointer;
    //typedef typename allocator_traits::reference reference; // one of our allocator_traits doesn't reveal this but I can't figure out which one
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    typedef typename allocator_type::value_type value_type;

protected:
    // NOTE: It's conceivable we could use a value_evaporator here in situations where
    // allocated array pointed to a static const(expr) * of some kind
    TImpl m_impl;

    static bool CONSTEXPR is_locking = internal::has_locking_tag<allocator_type>::value;

public:

    // Always try to avoid explicit locking and unlocking ... but sometimes
    // you gotta do it, so these are public
    value_type* lock(size_type pos = 0, size_type count = 0)
    {
        return &m_impl.lock(pos, count);
    }

    void unlock()
    {
        m_impl.unlock();
    }

    const value_type* clock(size_type pos = 0, size_type count = 0) const
    {
        return &m_impl.clock(pos, count);
    }

    void cunlock() const
    {
        m_impl.cunlock();
    }

protected:
    explicit allocated_array(allocator_type& alloc) :
        m_impl(alloc) {}

    allocated_array(const allocated_array& copy_from)
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
        = default;
#else
        : m_impl(copy_from.m_impl)
    {
    }
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    allocated_array(std::initializer_list<value_type> initlist)
    {
        pointer p = lock();

        estd::copy(initlist.begin(), initlist.end(), p);

        m_impl.size(initlist.size());

        unlock();
    }
#endif

    handle_with_offset offset(size_type  pos) const
    {
        //return helper.get_allocator().offset(helper.handle(), pos);
        return m_impl.offset(pos);
    }

    // internal version of replace not conforming to standard
    // (standard version also inserts or removes characters if requested,
    //  this one ONLY replaces the entire buffer)
    // TODO: change to assign
    void assign(const value_type* buf, size_type len)
    {
        m_impl.copy_into(buf, 0, len);
    }


    template <class TForeignImpl>
    bool starts_with(const allocated_array<TForeignImpl>& compare_to) const
    {
        const value_type* s = clock();
        const value_type* t = compare_to.clock();

        size_type source_max = size();
        size_type target_max = compare_to.size();

        while(source_max-- && target_max--)
            if(*s++ != *t++)
            {
                cunlock();
                compare_to.cunlock();
                return false;
            }

        cunlock();
        compare_to.cunlock();
        // if compare_to is longer than we are, then it's also a fail
        return source_max != -1;
    }

public:
    allocated_array() {}

    template <class THelperParam>
    allocated_array(const THelperParam& p) :
            m_impl(p) {}


    // TODO: make accessor do this comparison in a self contained way
    // allocator itself only needs to be stateful if it needs to do handle locking
    // if not, then we assume it's pointer based and thusly can access the item
    // without an allocator pointer, stateful or otherwise
    // TODO: Move this accessor typedef out to helper, who can lean slightly less on
    // constexpr to get the job done
#ifdef FEATURE_CPP_CONSTEXPR
    typedef typename estd::conditional<
#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                allocator_traits::is_stateful() && allocator_traits::is_locking(),
#else
                allocator_traits::is_stateful_exp && allocator_traits::is_locking_exp,
#endif
                estd::internal::accessor<allocator_type>,
                estd::internal::accessor_stateless<allocator_type> >::type
                accessor;
#else
    // pre C++11 tricky to optimize this way
    typedef estd::internal::accessor<allocator_type> accessor;
#endif


    class iterator
    {
    private:
        accessor current;

    public:
        // All-or-nothing, though not supposed to be that way till C++17 but is sometimes
        // before that (http://en.cppreference.com/w/cpp/iterator/iterator_traits)
        typedef typename allocator_type::value_type value_type;
        typedef int difference_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef ::std::forward_iterator_tag iterator_category;

        value_type& lock() { return current.lock(); }
        void unlock() { current.unlock(); }


        iterator(allocator_type& allocator, const handle_with_offset& h ) :
            current(allocator, h)
        {

        }

        iterator(const iterator& copy_from) : current(copy_from.current) {}

        ~iterator()
        {
            // FIX: not so great.  It might be passable, though not recommended,
            // to unlock an alread unlocked value.  But it's less certain what
            // happens when we only unlock once if many lock() calls have happened
            // -- another reason to consolidate with iterators/list, because it
            // attempts to deal with this a little bit with a lock counter
            unlock();
        }

        // prefix version
        iterator& operator++()
        {
            current.h_exp().increment();
            return *this;
        }


        iterator& operator--()
        {
            current.h_exp().increment(-1);
            return *this;
        }

        // postfix version
        iterator operator++(int)
        {
            iterator temp(*this);
            operator++();
            return temp;
        }

        // postfix version
        iterator operator--(int)
        {
            iterator temp(*this);
            operator--();
            return temp;
        }

        ptrdiff_t operator-(const iterator& subtrahend) const
        {
            return current.h_exp() - subtrahend.current.h_exp();
        }

        inline iterator operator+(ptrdiff_t offset)
        {
            iterator it(*this);

            it.current.h_exp() += offset;

            return it;
        }

        inline iterator operator-(ptrdiff_t offset)
        {
            iterator it(*this);

            it.current.h_exp() -= offset;

            return it;
        }

        bool operator>(const iterator& compare) const
        {
            return current.h_exp() > compare.current.h_exp();
        }


        /*
        bool operator<(const iterator& compare) const
        {
            return current < compare.current;
        } */


        // NOTE: Descrepency between doing a pointer-ish compare and a value compare
        bool operator==(const iterator& compare_to) const
        {
            return current.h_exp() == compare_to.current.h_exp();
        }

        bool operator!=(const iterator& compare_to) const
        {
            return !(operator ==)(compare_to);
            //return current != compare_to.current;
        }

        value_type& operator*()
        {
            // TODO: consolidate with InputIterator behavior from iterators/list.h
            return lock();
        }

        const value_type& operator*() const
        {
            return lock();
        }


        iterator& operator=(const iterator& copy_from)
        {
            //current = copy_from.current;
            new (&current) accessor(copy_from.current);
            return *this;
        }
    };

    typedef const iterator const_iterator;

    size_type size() const { return m_impl.size(); }

    allocator_type& get_allocator() const
    {
        // FIX: Hate this, but since we have stateful allocators flying around everywhere, even
        // embedded within dynamic array, we are forced to do this so that we conform to std::get_allocator
        // const call (important because we need to conform to const for front, back, etc)
        // because we need locking, side affects are to be expected
        return const_cast<impl_type&>(m_impl).get_allocator();
    }

    iterator begin()
    {
        return iterator(get_allocator(), offset(0));
    }

    iterator end()
    {
        handle_with_offset o = offset(size());

        return iterator(get_allocator(), o);
    }


    const_iterator cend() const
    {
        handle_with_offset o = offset(size());

        return iterator(get_allocator(), o);
    }

    const_iterator end() const
    {
        return cend();
    }

    accessor at(size_type pos) const
    {
        // TODO: place in error/bounds checking runtime ability within
        // accessor itself.  perhaps wrap it all up in a FEATURE_ESTD_BOUNDSCHECK
        // to compensate for lack of exceptions requiring additional
        // data in accessor itself
        return accessor(get_allocator(), offset(pos));
    }


    accessor operator[](size_type pos) const
    {
        return accessor(get_allocator(), offset(pos));
    }

    accessor front() const
    {
        return accessor(get_allocator(), offset(0));
    }

    accessor back() const
    {
        return accessor(get_allocator(), offset(size() - 1));
    }

    size_type max_size() const
    {
        return m_impl.max_size();
    }

    bool empty() const
    {
        return m_impl.empty();
    }

    // copy (into dest)
    // officially only for basic_string, but lives happily here in allocated_array
    size_type copy(typename estd::remove_const<value_type>::type* dest,
                   size_type count, size_type pos = 0) const
    {
        const value_type* src = clock();

        // TODO: since we aren't gonna throw an exception, determine what to do if
        // pos > size()

        if(pos + count > size())
            count = size() - pos;

        // TODO: Do the m_impl.copy_from here

        memcpy(dest, src + pos, count * sizeof(value_type));

        cunlock();

        return count;
    }

    template <class TForeignImpl>
    bool operator ==(const allocated_array<TForeignImpl>& compare_to) const
    {
        size_type raw_size = size();
        size_type s_size = compare_to.size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const value_type* raw = clock();
        const value_type* s = compare_to.clock();

        bool result = estd::equal(raw, raw + raw_size, s);

        cunlock();
        compare_to.cunlock();

        return result;
    }
};

// base class for dynamic_array
// performs a lot of dynamic_array duties, but behaves as a read-only entity
// though constness is not specifically enforced, as writable entities will
// derive from this
template <class THelper>
class runtime_array
{
public:
};

}}
