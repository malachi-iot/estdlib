/**
 *  @file
 */
#pragma once

#include "array.h"
#include "vector.h"
#include "functional.h"
#include "internal/deduce_fixed_size.h"
#include "internal/priority_queue.h"

// deviates from C++ standard queue in that a bool is returned to indicate
// if push/pop succeeds

namespace estd {

namespace experimental {

template <size_t N>
struct smalL_dequeue_policy
{
    static CONSTEXPR uint8_t size_bits = estd::internal::deduce_bit_count<N>::value;
    static CONSTEXPR ptrdiff_t size_max = 1 << (size_bits - 1);
    typedef typename estd::internal::deduce_fixed_size_t<size_max * 2>::size_type size_2_type;

    struct size_2_type_
    {
        size_2_type m_front : size_bits;
        size_2_type m_back : size_bits;
    } positions;

    template <class TArray>
    typename TArray::iterator front(TArray& a)
    {
        return &a[positions.m_front];
    }

    template <class TArray>
    typename TArray::iterator back(TArray& a)
    {
        return &a[positions.m_back];
    }
};


struct fast_dequeue_policy
{

};

}

namespace layer1 {

// NOTE: Since layer1 is always a fixed size, this is hard wired as a
// *circular* queue.  Otherwise we'd have a queue which goes to the
// end and basically always fails.  If that's really your desired behavior
// you can recreate that here by checking size() against max_size()
// NOTE: kind of general use and uses iterators straight from array.  Using size_type begin/end
// pos would be more memory efficient.  Eventually do that as an optimization, but even
// then keep this general use one around as it is arguably faster using direct pointers.
// For above, refactoring to standard deque<T, Allocator> would fit the bill - 'grow' type
// operations would merely compile-time fail [as they should] for fixed allocations
// TODO: Likely we'll want to mate this to a layer1::stack implementation, right
// now it seems they differ only in that stack has more of a constant m_front
template <class T, size_t N, class TPolicy = experimental::fast_dequeue_policy >
class deque : protected TPolicy
{
    typedef array<T, N> array_t;
    typedef array_t container_type;

    array_t m_array;

    // TODO: Make special iterator for dequeue which does the rollover/rollunder
    // and checks that you only move back and forth within one 'session' of
    // elements
    typedef typename array_t::iterator array_iterator;

    // front aka head aka 'leftmost' part of array,
    //   where items are traditionally retrieved
    // back aka tail aka 'rightmost' part of array,
    //   where items are traditionally added
    array_iterator m_front;
    array_iterator m_back;

    // an easy, but not the only, use case is where m_back is more
    // to the right than m_front, indicating that m_back - m_front
    // items are present, starting from m_front
    // alternatively, m_back might be *before* m_front, indicating
    // 0-m_back + m_fromt-end = range of items


    // because I don't want to waste a slot
    bool m_empty;

    // called when i is incremented, evaluates if i reaches rollover point
    // and if so points it back at the beginning
    void evaluate_rollover(array_iterator* i) const
    {
        if(*i == m_array.end())
            // FIX: cleanup const abuse
            *i = (array_iterator)m_array.begin();
    }

    // called when i is decremented, opposite of rollover check
    // returns true when a rollover is detected.  Returns whether
    // we rolled over to help with decrement (again, artifact
    // of no cbegin)
    bool evaluate_rollunder(array_iterator* i) const
    {
        if(*i == m_array.begin())
        {
            // FIX: cleanup const abuse
            *i = (array_iterator)m_array.end();
            return true;
        }
        else
            return false;
    }

    // have to do these increment/decrements out here because array iterator itself
    // wouldn't handle rollovers/rollunders
    void decrement(array_iterator* i) const
    {
        // doing i-- after because we don't have a 'before begin' iterator
        evaluate_rollunder(i);
        //if(!evaluate_rollunder(i))
            (*i)--;
    }

    // handle rollover and empty-flag set for iterator
    void increment(array_iterator* i) const //, bool empty = false)
    {
        (*i)++;
        evaluate_rollover(i);
        //m_empty = empty;
    }

    void increment(array_iterator* i, bool empty)
    {
        increment(i);
        m_empty = empty;
    }

    // FIX: Something is wrong, m_front
    // never registers as > m_back.  Might just
    // be permissive test?
    size_t _size() const
    {
        if(m_front > m_back)
            return N - (m_front - m_back);
        else
            return m_back - m_front;
    }

public:
    deque() :
        m_front(m_array.begin()),
        m_back(m_array.begin()),
        m_empty(true)
    {
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    constexpr deque(deque&& move_from) :
        m_array(std::move(move_from.m_array)),
        m_front(std::move(move_from.m_front)),
        m_back(std::move(move_from.m_back)),
        m_empty(std::move(move_from.m_empty))
    {}
#endif
    

    ~deque()
    {
        // TODO: destruct all remaining items
    }

    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef reference accessor;
    typedef typename container_type::size_type size_type;


    // THIS iterator will do rollovers.  array_iterator won't
    struct forward_iterator
    {
        array_iterator current;
        const deque& parent;

        forward_iterator(const deque& parent, array_iterator current)
            : parent(parent),
              current(current)
        {}

        forward_iterator& operator++()
        {
            parent.increment(&current);
            return *this;
        }

        // FIX: perhaps we can do reverse iteration easily here after all
        forward_iterator& operator--()
        {
            parent.decrement(&current);
            return *this;
        }

        forward_iterator operator++(int)
        {
            forward_iterator retval = *this;
            ++(*this);
            return retval;
        }

        reference operator *()
        {
            return *current;
        }

        bool operator==(const forward_iterator& compare_to)
        {
            return compare_to.current == current;
        }

        bool operator!=(const forward_iterator& compare_to)
        {
            return compare_to.current != current;
        }
    };

    typedef forward_iterator iterator;
    typedef const iterator const_iterator;

    iterator begin() { return iterator(*this, m_front); }

    iterator end() { return iterator(*this, m_back); }

    bool empty() const { return m_empty; }

    size_type size() const
    {
        if(empty()) return 0;

        size_type s = _size();

        if(s == 0) s = max_size();

        return s;
    }

    size_type max_size() const { return m_array.size(); }

    bool push_back(const_reference value)
    {
        *m_back++ = value;
        m_empty = false;

        evaluate_rollover(&m_back);

        return true;
    }


    ///
    /// \brief prepend element to the double-ended queue
    ///
    /// this means internal iterator moves left and evaluates a rollunder
    ///
    /// \param value
    /// \return
    ///
    bool push_front(const_reference value)
    {
        // NOTE: doesn't do a before-begin style, but for consistency probably should
        evaluate_rollunder(&m_front);
        m_front--; // due to the non-before-begin approach. confusing

        *m_front = value;

        m_empty = false;

        return true;
    }


    bool pop_back()
    {
        back().~value_type();

        decrement(&m_back);

        if(m_front == m_back) m_empty = true;

        return true;
    }

    bool pop_front()
    {
        front().~value_type();

        m_front++;

        evaluate_rollover(&m_front);

        if(m_front == m_back) m_empty = true;

        return true;
    }

    reference front()
    {
        array_iterator i = m_front;

        //increment(&i);

        return *i;
    }

    const_reference front() const
    {
        array_iterator i = m_front;

        //increment(&i);

        return *i;
    }

    reference back()
    {
        array_iterator i = m_back;

        decrement(&i);

        return *i;
    }

    const_reference back() const
    {
        array_iterator i = m_back;

        decrement(&i);

        return *i;
    }

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    // Compiles but untested
    template <class ...TArgs>
    reference emplace_front(TArgs&&...args)
    {
        value_type& retval = *m_front;

        new (&retval) value_type(std::forward<TArgs>(args)...);

        decrement(&m_front);
        m_empty = false;

        return retval;
    }

    // Compiles but untested
    template <class ...TArgs>
    reference emplace_back(TArgs&&...args)
    {
        value_type& retval = *m_back;

        new (&retval) value_type(std::forward<TArgs>(args)...);

        increment(&m_back, false);

        return retval;
    }
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
    bool push_back(value_type&& value)
    {
        value_type& back = *m_back++;

        new (&back) value_type(std::move(value));

        evaluate_rollover(&m_back);
        m_empty = false;

        return true;
    }
#endif
};




}

// http://en.cppreference.com/w/cpp/container/queue
// we might do just as well using std::queue here, it's mainly the containers which do all the magic
template <class T, class Container>
class queue
{
    Container c;

public:
#ifdef FEATURE_CPP_MOVESEMANTIC
    constexpr queue(queue&& move_from) : c(std::move(move_from.c)) {}

    queue() {}
#endif
    
    typedef typename Container::value_type value_type;
    typedef typename Container::const_reference const_reference;
    typedef typename Container::reference reference;
    typedef typename Container::size_type size_type;
    typedef Container container_type;

    bool empty() const { return c.empty(); }

    size_type size() const { return c.size(); }

    bool pop() { return c.pop_front(); }

    bool push(const_reference value)
    {
        return c.push_back(value);
    }

    reference front() { return c.front(); }

    const_reference front() const { return c.front(); }

    reference back() { return c.back(); }

    const_reference back() const { return c.back(); }

#ifdef FEATURE_CPP_VARIADIC
    // emplaces at the back, as per spec
    template <class ...TArgs>
    reference emplace(TArgs&&... args)
    {
        return c.emplace_back(std::forward<TArgs>(args)...);
    }
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
    bool push(value_type&& value)
    {
        return c.push_back(std::move(value));
    }
#endif
};

namespace layer1 {

template <class T, size_t size>
#ifdef FEATURE_CPP_ALIASTEMPLATE
using queue = estd::queue<T, layer1::deque<T, size> >;
#else
class queue : public estd::queue<T, layer1::deque<T, size> >
{
};
#endif

}

}
