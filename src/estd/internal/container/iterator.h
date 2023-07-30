#pragma once

#include "../platform.h"
#include "../iterator_standalone.h"

#include "fwd.h"

namespace estd { namespace internal {

struct locking_iterator_modes
{
    enum _
    {
        /// read-only: lock is required (& automatically performed), but unlock isn't.  data is implicitly always pinned
        ro,
        /// locking is just a formmality and resolves to a pointer dereference.
        /// unlock is a noop and is treated as such
        shim,
        /// implicit casting to reference is disabled.  Only explicit lock() call reveals reference
        _explicit,
        /// iterator locks and unlocks as it comes and goes from scope
        auto_instance,
        /// iterator locks and unlocks per element it iterates over
        auto_increment
    };
};


template <class Allocator, locking_iterator_modes::_>
struct locking_iterator_base {};


// Iterator for handle accessor types:
// - traditional_iterator
// - accessor_shared
// Informal concept:
// - lock() / unlock() method
// - h_exp() access to pointer-like (for address manipulation only)
// Handle accessors attempt to delegate control of locking/pinning to itself
template <class Allocator, class Accessor,
          locking_iterator_modes::_ mode = locking_iterator_modes::shim>
class locking_iterator : public locking_iterator_base<Allocator, mode>
{
    typedef locking_iterator this_type;

public:
    typedef Allocator allocator_type;
    typedef Accessor accessor;

private:
    accessor current;

public:
    // All-or-nothing, though not supposed to be that way till C++17 but is sometimes
    // before that (http://en.cppreference.com/w/cpp/iterator/iterator_traits)
    typedef typename allocator_type::value_type value_type;
    typedef int difference_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    // DEBT: estd-ize this tag, aliasing ::std ones when possible
    typedef ::std::bidirectional_iterator_tag iterator_category;
    typedef typename accessor::locked_type locked_type;
    typedef typename accessor::const_locked_type const_locked_type;

    locked_type lock() { return current.lock(); }
    const_locked_type lock() const { return current.clock(); }
    void unlock() { current.unlock(); }


    ESTD_CPP_CONSTEXPR_RET EXPLICIT locking_iterator(const accessor& a) :
        current(a)
    {

    }

    locking_iterator(const this_type& copy_from) : current(copy_from.current) {}

    ~locking_iterator()
    {
        // FIX: not so great.  It might be passable, though not recommended,
        // to unlock an alread unlocked value.  But it's less certain what
        // happens when we only unlock once if many lock() calls have happened
        // -- another reason to consolidate with iterators/list, because it
        // attempts to deal with this a little bit with a lock counter
        unlock();
    }

    // prefix version
    this_type& operator++()
    {
        current.h_exp() += 1;
        return *this;
    }


    this_type& operator--()
    {
        current.h_exp() += -1;
        return *this;
    }

    // postfix version
    this_type operator++(int)
    {
        this_type temp(*this);
        operator++();
        return temp;
    }

    // postfix version
    this_type operator--(int)
    {
        this_type temp(*this);
        operator--();
        return temp;
    }

    ptrdiff_t operator-(const this_type& subtrahend) const
    {
        return current.h_exp() - subtrahend.current.h_exp();
    }

    this_type& operator += (ptrdiff_t offset)
    {
        current.h_exp() += offset;
        return *this;
    }

    this_type& operator -= (ptrdiff_t offset)
    {
        current.h_exp() -= offset;
        return *this;
    }

    inline this_type operator+(ptrdiff_t offset) const
    {
        this_type it(*this);

        it += offset;

        return it;
    }

    inline this_type operator-(ptrdiff_t offset)
    {
        this_type it(*this);

        it.current.h_exp() -= offset;

        return it;
    }

    ESTD_CPP_CONSTEXPR_RET bool operator>(const this_type& compare) const
    {
        return current.h_exp() > compare.current.h_exp();
    }

    ESTD_CPP_CONSTEXPR_RET bool operator>=(const this_type& compare) const
    {
        return current.h_exp() >= compare.current.h_exp();
    }

    ESTD_CPP_CONSTEXPR_RET bool operator<(const this_type& compare) const
    {
        return current.h_exp() < compare.current.h_exp();
    }

    /*
    bool operator<(const iterator& compare) const
    {
        return current < compare.current;
    } */


    // NOTE: Discrepancy between doing a pointer-ish compare and a value compare
    ESTD_CPP_CONSTEXPR_RET bool operator==(const this_type& compare_to) const
    {
        return current.h_exp() == compare_to.current.h_exp();
    }

    ESTD_CPP_CONSTEXPR_RET bool operator!=(const this_type& compare_to) const
    {
        return !(operator ==)(compare_to);
        //return current != compare_to.current;
    }

    /*
    handle_accessor operator*()
    {
        // TODO: consolidate with InputIterator behavior from iterators/list.h
        return current;
    }

    const handle_accessor operator*() const
    {
        return current;
    }   */

    // TODO: In the future, we may want to return accessor here directly to further defer
    // lock/unlock operation.  For now, we have no true locking data stores.

    locked_type operator*() { return current.lock(); }
    const_locked_type operator*() const { return current.clock(); }

    const value_type* operator->() const { return &current.clock(); }

    this_type& operator=(const this_type& copy_from)
    {
        current = copy_from.current;
        //new (&current) accessor(copy_from.current);
        return *this;
    }
};


}}
