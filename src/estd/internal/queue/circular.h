#pragma once

#include "../../array.h"


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
            || is_trivial<T>::value
#endif
            ;

        using uninitialized_array = internal::array<impl::uninitialized_array<T, N>>;

        using container_type = conditional_t<is_trivial,
            T[N],
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
    using array_policy = typename Policy::template array<T, N>;
    using container_type = typename array_policy::container_type;

    container_type array_;
};

template <class T, unsigned N, class Policy, class Enabled = void>
class circular_queue_base : public circular_queue_container_base<T, N, Policy>
{
protected:
};


template <class T, unsigned N, class Policy>
class circular_queue_base<T, N, Policy, enable_if_t<Policy::type == queue_options::flagged>> :
    public circular_queue_container_base<T, N, Policy>
{
protected:
    bool empty_{true};

};


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

    using container_type = typename array_policy::container_type;
    using iterator_type = typename array_policy::iterator_type;

public:
};



}}