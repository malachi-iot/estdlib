#pragma once

#include "../../array.h"
#include "../../atomic.h"

namespace estd { namespace internal {

enum class queue_options
{
    sentinel    = 0x0001,
    bare        = 0x0002,
    flagged     = 0x0003,
    counter     = 0x0004,
    mask        = 0x0007,

    atomic      = 0x0008,

    // force trivial behavior i.e. calls default constructor
    trivial     = 0x0010,

    default_opt = flagged
};

ESTD_FLAGS(queue_options)

template <queue_options o>
struct dequeue_policy
{
    // consolidate and use map nullable things.  Note I get the feeling there's already a std mechanism for that
    using nullable = void;

    static constexpr queue_options type = o & queue_options::mask;
    static constexpr bool atomic = o & queue_options::atomic;

    template <class T, unsigned N>
    struct array
    {
        constexpr static bool is_trivial = is_set(o & queue_options::trivial) || is_integral<T>::value
#if FEATURE_ESTD_IS_TRIVIAL
            || estd::is_trivial<T>::value
#endif
            ;

        using uninitialized_array = internal::array<impl::uninitialized_array<T, N>>;

        // NOTE: Wanted to use raw array - for that, gymnastics are required to get at begin/end
        // with iterator sensibilities though.
        using container_type = conditional_t<is_trivial,
            estd::array<T, N>,
            uninitialized_array>;

        using iterator_type = conditional_t<is_trivial,
            T*, typename uninitialized_array::iterator_type>;

        using const_iterator_type = conditional_t<is_trivial,
            const T*, typename uninitialized_array::const_iterator_type>;
    };
};


template <class T, unsigned N, class Policy>
class circular_queue_container_base
{
protected:
    constexpr static bool atomic = Policy::atomic;

    using array_policy = typename Policy::template array<T, N>;
    using container_type = typename array_policy::container_type;
    using iterator = typename array_policy::iterator_type;
    using const_iterator = typename array_policy::const_iterator_type;

    container_type array_;

    // front aka head aka 'leftmost' part of array,
    //   where items are traditionally retrieved
    // back aka tail aka 'rightmost' part of array,
    //   where items are traditionally added
    iterator front_, back_;

    // called when i is incremented, evaluates if i reaches rollover point
    // and if so points it back at the beginning
    void evaluate_rollover(const_iterator* i) const
    {
        if(*i == array_.end())
            *i = array_.begin();
    }

    // called when i is decremented, opposite of rollover check
    // returns true when a rollover is detected.  Returns whether
    // we rolled over to help with decrement
    bool evaluate_rollunder(const_iterator* i) const
    {
        if(*i != array_.begin()) return false;

        *i = array_.end();
        return true;
    }

    // have to do these increment/decrements out here because array iterator itself
    // wouldn't handle rollovers/rollunders
    void decrement(const_iterator* i) const
    {
        // doing i-- after because we don't have a 'before begin' iterator
        evaluate_rollunder(i);
        --(*i);
    }

    void increment(const_iterator* i) const
    {
        ++(*i);
        evaluate_rollover(i);
    }
};

template <class T, unsigned N, class Policy, class Enabled = void>
class circular_queue_base : public circular_queue_container_base<T, N, Policy>
{
protected:
};


template <class T, unsigned N, class Policy>
class circular_queue_base<T, N, Policy, enable_if_t<Policy::atomic == queue_options::flagged>> :
    public circular_queue_container_base<T, N, Policy>
{
protected:
    bool empty_{true};

};


#if FEATURE_STD_ATOMIC
template <class T, unsigned N, class Policy>
class circular_queue_base<
    T, N, Policy, enable_if_t<
        (Policy::type == queue_options::bare || Policy::type == queue_options::sentinel) &&
        Policy::atomic>> :
    public circular_queue_container_base<T, N, Policy>
{
    using base_type = circular_queue_container_base<T, N, Policy>;
    using typename base_type::iterator;
    using typename base_type::const_iterator;

protected:
    std::atomic<iterator> front_, back_;
};
#endif

template <class T, unsigned N, class Policy>
class circular_queue_base<T, N, Policy, enable_if_t<Policy::type == queue_options::counter>> :
    public circular_queue_container_base<T, N, Policy>
{
protected:
    unsigned counter_{};
};


template <class T, unsigned N, class Policy = dequeue_policy<queue_options::default_opt>>
class circular_queue : public circular_queue_base<T, N, Policy>
{
    using base_type = circular_queue_base<T, N, Policy>;
    using typename base_type::array_policy;
    using typename base_type::container_type;
    using typename base_type::iterator;

    using const_reference = const T&;

    using base_type::front_;
    using base_type::back_;

public:
    bool push_back(const_reference value)
    {
        *back_++ = value;
        //m_empty = false;

        base_type::evaluate_rollover(back_);

        return true;
    }


};



}}