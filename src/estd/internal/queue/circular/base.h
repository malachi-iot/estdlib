#pragma once

#include "../../type_traits.h"
#include "../../../atomic.h"

#include "enum.h"
#include "fwd.h"

namespace estd { namespace internal {

template <class Policy>
class circular_queue_container_base
{
protected:
    constexpr static bool atomic = Policy::atomic;
    constexpr static queue_options type = Policy::type;
    //constexpr static bool atomic = true;
    static constexpr bool no_rollover = is_set(Policy::options & queue_options::no_rollover);

    using container_policy = Policy;
    using container_type = typename container_policy::container_type;
    using value_type = typename container_type::value_type;
#if FEATURE_STD_ATOMIC
    // 08SEP25 MB DEBT: std::atomic defaults to memory_order_seq_cst, when for us it's likely
    // memory_order_acquire/memory_order_release is faster and sufficient.
    // As per https://codesignal.com/learn/courses/lock-free-concurrent-data-structures/lessons/an-introduction-to-memory-ordering-and-atomic-operations
    using iterator = conditional_t<atomic,
        std::atomic<typename container_policy::iterator_type>,
        typename container_policy::iterator_type>;
    using const_iterator = conditional_t<atomic,
        std::atomic<typename container_policy::const_iterator_type>,
        typename container_policy::const_iterator_type>;
#else
    using iterator = typename container_policy::iterator_type;
    using const_iterator = typename container_policy::const_iterator_type;
#endif

    container_type array_;

    // front aka head aka 'leftmost' part of array,
    //   where items are traditionally retrieved
    // back aka tail aka 'rightmost' part of array,
    //   where items are traditionally added
    // https://softwareengineering.stackexchange.com/questions/144477/on-a-queue-which-end-is-the-head
    // Although we are somewhat ambidextrous, the paradigm is:
    // front = first = begin, back = last = end with [begin...end) so
    // we tune accordingly with back_ always as the first empty slot
    // past the occupied slot
    iterator front_, back_;

    // called when i is incremented, evaluates if i reaches rollover point
    // and if so points it back at the beginning
    // DEBT: const all this up.  const** is super tricky in this instance
    template <class ItPtr>
    ESTD_CPP_CONSTEXPR(14) void evaluate_rollover(ItPtr i)
    {
        if(*i == array_.end())
            *i = array_.begin();
    }

    // called when i is decremented, opposite of rollover check
    // returns true when a rollover is detected.  Returns whether
    // we rolled over to help with decrement
    template <class ItPtr>
    ESTD_CPP_CONSTEXPR(14) bool evaluate_rollunder(ItPtr i)
    {
        if(*i != array_.begin()) return false;

        *i = array_.end();
        return true;
    }

    // have to do these increment/decrements out here because array iterator itself
    // wouldn't handle rollovers/rollunders
    // DEBT: const all this up.  const** is super tricky in this instance
    template <class ItPtr>
    ESTD_CPP_CONSTEXPR(14) void decrement(ItPtr i)
    {
        // doing i-- after because we don't have a 'before begin' iterator
        evaluate_rollunder(i);
        --(*i);
    }

    // DEBT: const all this up.  const** is super tricky in this instance
    template <class ItPtr>
    ESTD_CPP_CONSTEXPR(14) void increment(ItPtr i)
    {
        ++(*i);
        evaluate_rollover(i);
    }

    static ESTD_CPP_CONSTEVAL bool increment_size() { return{}; }
    static ESTD_CPP_CONSTEVAL bool decrement_size() { return{}; }

    template <class ...Args>
    constexpr explicit circular_queue_container_base(Args&&...args) :
        array_(std::forward<Args>(args)...),
        front_{&array_[0]},
        back_{&array_[0]}
    {
    }

    class pos_iterator_base
    {
    protected:
        static ESTD_CPP_CONSTEVAL bool bump_up() { return{}; }
        static ESTD_CPP_CONSTEVAL bool bump_down() { return{}; }
    };

    class iterator_base
    {
    protected:
        static ESTD_CPP_CONSTEVAL bool bump_up() { return{}; }
        static ESTD_CPP_CONSTEVAL bool bump_down() { return{}; }
    };

public:
    using size_type = unsigned;

    constexpr size_type max_size() const
    {
        return array_.size() - (type == queue_options::sentinel ? 1 : 0);
    }

    value_type& front()
    {
        return *front_;
    }

    constexpr const value_type& front() const
    {
        return *front_;
    }
};


}}