#pragma once

#include "../../type_traits.h"
#include "../../../atomic.h"

#include "enum.h"
#include "fwd.h"

namespace estd { namespace internal {

// DEBT: Split out the 'container' part of this base from the front/back tracking,
// namely in support of 'packed' and *maybe* 'bare'
template <ESTD_CPP_CONCEPT(concepts::v1::CircularQueuePolicy) Policy>
class circular_queue_container_base
{
    using this_type = circular_queue_container_base;

protected:
    static constexpr bool atomic = is_set(Policy::options & queue_options::atomic);
    static constexpr queue_options type = Policy::type;
    //constexpr static bool atomic = true;
    static constexpr bool no_rollover = is_set(Policy::options & queue_options::no_rollover);
    static constexpr bool strict = is_set(Policy::options & queue_options::strict);
    static constexpr bool hardened = is_set(Policy::options & queue_options::hardened);

    static_assert(!(strict && hardened), "strict and hardened must not both be selected");

    using container_policy = Policy;
    using container_type = typename container_policy::container_type;

    ESTD_CPP_STD_VALUE_TYPE(typename container_type::value_type)

#if FEATURE_STD_ATOMIC
    // 08SEP25 MB DEBT: std::atomic defaults to memory_order_seq_cst, when for us it's likely
    // memory_order_acquire/memory_order_release is faster and sufficient.
    // As per https://codesignal.com/learn/courses/lock-free-concurrent-data-structures/lessons/an-introduction-to-memory-ordering-and-atomic-operations
    using container_it = conditional_t<atomic,
        std::atomic<typename container_policy::iterator_type>,
        typename container_policy::iterator_type>;
    using const_container_it = conditional_t<atomic,
        std::atomic<typename container_policy::const_iterator_type>,
        typename container_policy::const_iterator_type>;
#else
    using container_it = typename container_policy::iterator_type;
    using const_container_it = typename container_policy::const_iterator_type;
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
    container_it front_, back_;

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

#if UNIT_TESTING
public:
#endif
    ESTD_CPP_CONSTEXPR(14) unsigned normalize_pos(unsigned pos) const
    {
        const unsigned front_idx = front_ - &array_[0];

        // DEBT: When size() is power of 2, do bitwise modulo optimization ala
        // https://asawicki.info/news_1433_bit_tricks_with_modulo
        // Note that constexpr of 'array_.size()' probably spurs this compile
        // time optimization, so it's debatable how important explicitly doing
        // it really is.

        pos += front_idx;
        pos %= array_.size();

        if(hardened)  assert(pos < max_size());

        return pos;
    }

    constexpr circular_queue_container_base() :
        front_{&array_[0]},
        back_{&array_[0]}
    {}

    ESTD_CPP_DEFAULT_RULE_OF_5(circular_queue_container_base)

    template <class ...Args>
    constexpr explicit circular_queue_container_base(in_place_t, Args&&...args) :
        array_(std::forward<Args>(args)...),
        front_{&array_[0]},
        back_{&array_[0]}
    {
    }

    class iterator
    {
        using this_type = iterator;

    protected:
        using parent_type = circular_queue_container_base;

        ESTD_CPP_CONSTEXPR(14) void bump(true_type)
        {
            parent_.increment(&current_);
        }

        ESTD_CPP_CONSTEXPR(14) void bump(false_type)
        {
            parent_.decrement(&current_);
        }

        parent_type& parent_;
        pointer current_;

        constexpr explicit iterator(parent_type& parent, pointer current, size_t = 0) :
            parent_{parent},
            current_{current}
        {}

    public:
        reference operator*() { return *current_; }
        constexpr const_reference operator*() const { return *current_; }

        pointer operator->() { return current_; }
        constexpr const_pointer operator->() const { return current_; }

        constexpr bool operator==(const this_type& other) const
        {
            return current_ == other.current_;
        }

        constexpr bool operator!=(const this_type& other) const
        {
            return current_ != other.current_;
        }
    };

    class pos_iterator : public iterator
    {
        // In reverse mode, pos is +1 offset so as to not need signed for -1 rend position
        unsigned pos_;
        using this_type = pos_iterator;

    protected:
        using parent_type = circular_queue_container_base;

        ESTD_CPP_CONSTEXPR(14) void bump(true_type)
        {
            iterator::bump(true_type{});
            ++pos_;
        }

        ESTD_CPP_CONSTEXPR(14) void bump(false_type)
        {
            iterator::bump(false_type{});
            --pos_;
        }

        constexpr explicit pos_iterator(parent_type& parent, pointer current, size_t pos) :
            iterator(parent, current),
            pos_{static_cast<unsigned>(pos)}
        {}

    public:
        constexpr bool operator==(const this_type& other) const
        {
            return pos_ == other.pos_;
        }

        constexpr bool operator!=(const this_type& other) const
        {
            return pos_ != other.pos_;
        }
    };

    ESTD_CPP_CONSTEXPR(14) void clear()
    {
        front_ = back_ = &array_[0];
    }

public:
    using size_type = unsigned;

    constexpr size_type max_size() const
    {
        return array_.size() - (type == queue_options::sentinel ? 1 : 0);
    }

    ESTD_CPP_CONSTEXPR(14) reference front()
    {
        return *front_;
    }

    constexpr const_reference front() const
    {
        return *front_;
    }

    ESTD_CPP_CONSTEXPR(14) reference operator[](size_type pos)
    {
        return array_[normalize_pos(pos)];
    }

    constexpr const_reference operator[](size_type pos) const
    {
        return array_[normalize_pos(pos)];
    }
};


}}