#pragma once

// DEBT: Consider moving this to container/queue/

#include "../../array.h"
#include "../../atomic.h"
#include "../../span.h"
#include "../container/unordered/traits.h"

// Diagrams at https://drive.google.com/file/d/10WeFACvoEOZzTeRIDI_unXnSP5WJqY9f
// DEBT: Above link is clunky... make it more directly go to diagrams

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
    // reject-on-full mode (needed for full lock-free behavior)
    no_rollover = 0x0020,

    default_opt = flagged
};

ESTD_FLAGS(queue_options)


template <class T, queue_options o, class Nullable = nullable_traits<T>>
struct circular_policy
{
    static constexpr queue_options type = o & queue_options::mask;
    static constexpr bool atomic = o & queue_options::atomic;
    constexpr static bool is_trivial = is_set(o & queue_options::trivial) || is_integral<T>::value
#if FEATURE_ESTD_IS_TRIVIAL
        || estd::is_trivial<T>::value
#endif
        ;

    using nullable = Nullable;
    static constexpr queue_options options = o;
};

template <class T, unsigned N, queue_options o>
struct array_circular_policy : circular_policy<T, o>
{
    using base_type = circular_policy<T, o>;
    using base_type::is_trivial;

    using uninitialized_array = internal::array<impl::uninitialized_array<T, N>>;

    // NOTE: Wanted to use raw array - for that, gymnastics are required to get at begin/end
    // with iterator sensibilities though.
    using container_type = conditional_t<is_trivial,
        estd::array<T, N>,
        uninitialized_array>;

    using iterator_type = conditional_t<is_trivial,
        T*, typename uninitialized_array::iterator>;

    using const_iterator_type = conditional_t<is_trivial,
        const T*, typename uninitialized_array::const_iterator>;
};

template <class T, unsigned N, queue_options o>
struct span_circular_policy : circular_policy<T, o>
{
    using container_type = span<T, N>;
    using iterator_type = typename container_type::iterator;
    using const_iterator_type = typename container_type::const_iterator;
};


template <class Policy>
class circular_queue_container_base
{
protected:
    constexpr static bool atomic = Policy::atomic;
    constexpr static queue_options type = Policy::type;
    //constexpr static bool atomic = true;

    using container_policy = Policy;
    using container_type = typename container_policy::container_type;
    using value_type = typename container_type::value_type;
#if FEATURE_STD_ATOMIC
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
    void evaluate_rollover(ItPtr i)
    {
        if(*i == array_.end())
            *i = array_.begin();
    }

    // called when i is decremented, opposite of rollover check
    // returns true when a rollover is detected.  Returns whether
    // we rolled over to help with decrement
    template <class ItPtr>
    bool evaluate_rollunder(ItPtr i)
    {
        if(*i != array_.begin()) return false;

        *i = array_.end();
        return true;
    }

    // have to do these increment/decrements out here because array iterator itself
    // wouldn't handle rollovers/rollunders
    // DEBT: const all this up.  const** is super tricky in this instance
    template <class ItPtr>
    void decrement(ItPtr i)
    {
        // doing i-- after because we don't have a 'before begin' iterator
        evaluate_rollunder(i);
        --(*i);
    }

    // DEBT: const all this up.  const** is super tricky in this instance
    template <class ItPtr>
    void increment(ItPtr i)
    {
        ++(*i);
        evaluate_rollover(i);
    }

    static ESTD_CPP_CONSTEVAL bool increment_size() { return{}; }
    static ESTD_CPP_CONSTEVAL bool decrement_size() { return{}; }

    constexpr circular_queue_container_base() :
        front_{&array_[0]},
        back_{&array_[0]}
    {
    }

public:
    constexpr size_t max_size() const { return array_.max_size(); }

    // DEBT: Need to handle sentinel flavor too - in fact, accidentally depends on it
    // DEBT: Troubled in other unidentified ways...
    size_t size() const
    {
        if(front_ > back_)
            return array_.max_size() - (front_ - back_);
        else
            return back_ - front_;
    }

    constexpr bool empty() const
    {
        // DEBT: Assumes sentinel mode
        return front_ == back_;
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

template <class Policy, class Enabled = void>
class circular_queue_base : public circular_queue_container_base<Policy>
{
protected:
};


template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::flagged>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

protected:
    using base_type::array_;
    using base_type::front_;
    using base_type::back_;

    bool empty_{true};

    ESTD_CPP_CONSTEXPR(14) void increment_size()
    {
        empty_ = false;
    }

    ESTD_CPP_CONSTEXPR(14) void decrement_size()
    {
        // Untested
        empty_ = base_type::back_ == base_type::front_;
    }

public:
    constexpr bool empty() const { return empty_; }

    size_t size() const
    {
        if(empty_) return 0;

        if(front_ >= back_)
            return array_.max_size() - (front_ - back_);
        else
            return back_ - front_;
    }
};


#if FEATURE_STD_ATOMIC_UNUSED
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

template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::counter>> :
    public circular_queue_container_base<Policy>
{
protected:
    unsigned size_{};

    ESTD_CPP_CONSTEXPR(14) void decrement_size()
    {
        --size_;
    }

    ESTD_CPP_CONSTEXPR(14) void increment_size()
    {
        ++size_;
    }

public:
    constexpr bool empty() const { return size_ == 0; }
    constexpr unsigned size() const { return size_; }
};


template <class Policy>
class circular_queue : public circular_queue_base<Policy>
{
    using base_type = circular_queue_base<Policy>;
    using typename base_type::container_policy;
    using typename base_type::container_type;

    ESTD_CPP_STD_VALUE_TYPE(typename container_type::value_type)

    using base_type::array_;
    using base_type::front_;
    using base_type::back_;

    using base_type::increment;
    using base_type::decrement;

    static constexpr bool no_rollover = Policy::options & queue_options::no_rollover;

    template <bool forward>
    class iterator_base
    {
        circular_queue& parent_;
        pointer current_;

        void bump(true_type) { parent_.increment(current_); }
        void bump(false_type) { parent_.decrement(current_); }

    public:
        constexpr explicit iterator_base(
            circular_queue& parent,
            pointer current) :
            parent_{parent},
            current_{current}
        {

        }

        iterator_base& operator++()
        {
            ESTD_CPP_IF_CONSTEXPR (forward)
                parent_.increment(&current_);
            else
                parent_.decrement(&current_);

            return *this;
        }

        iterator_base operator++(int)
        {
            iterator_base temp = *this;
            operator++();
            return temp;
        }

        iterator_base& operator--()
        {
            ESTD_CPP_IF_CONSTEXPR (forward)
                parent_.decrement(&current_);
            else
                parent_.increment(&current_);

            return *this;
        }

        reference operator*() { return *current_; }
        constexpr const_reference operator*() const { return *current_; }

        const_pointer operator->() const { return current_; }
    };

public:
    using base_type::type;
    using base_type::empty;

    constexpr circular_queue() = default;

    using iterator = iterator_base<true>;
    using reverse_iterator = iterator_base<false>;
    using const_iterator = const iterator;

    iterator begin()
    {
        return iterator{*this, front_};
    }

    iterator end()
    {
        return iterator{*this, back_};
    }

    constexpr const_iterator end() const
    {
        return iterator{*this, back_};
    }

    reference back()
    {
        pointer i = back_;

        decrement(&i);

        return *i;
    }

    pointer push_back_begin() { return back_; }
    bool push_back_end()
    {
        if(type != queue_options::sentinel)
        {
            const bool full = !empty() && back_ == front_;
            if(full)
            {
                if(no_rollover) return false;

                // rollover
                (*back_).~value_type();
                increment(&front_);
            }
            else
                base_type::increment_size();
        }

        increment(&back_);

        // NOTE: Not quite atomic, right ... ?
        if(type == queue_options::sentinel)
        {
            if(back_ == front_)
            {
                if(no_rollover) return false;

                // rollover
                (*back_).~value_type();
                increment(&front_);
            }
            else
                base_type::increment_size();
        }

        return true;
    }

    bool push_back(const_reference value)
    {
        *back_ = value;

        push_back_end();

        return true;
    }

    pointer push_front_begin()
    {
        if(type != queue_options::sentinel)
        {
            // UNTESTED
            const bool full = !empty() && back_ == front_;
            if(full)
            {
                if(no_rollover) return nullptr;

                // rollunder
                (*front_).~value_type();
                decrement(&back_);
            }
            else
                base_type::increment_size();
        }

        decrement(&front_);

        if(type == queue_options::sentinel)
        {
            if(front_ == back_)
            {
                if(no_rollover) return nullptr;

                // rollunder
                (*front_).~value_type();
                decrement(&back_);
            }
            else
                base_type::increment_size();
        }

        return front_;
    }

    void push_front(const_reference value)
    {
        push_front_begin();

        *front_ = value;
    }

    template <class ...Args>
    void emplace_front(Args&&...args)
    {
        new (push_front_begin()) value_type(std::forward<Args>(args)...);
    }


    template <class ...Args>
    void emplace_back(Args&&...args)
    {
        new (back_) value_type(std::forward<Args>(args)...);
        push_back_end();
    }


    void pop_front()
    {
        (*front_).~value_type();

        increment(&front_);
        base_type::decrement_size();
    }

    void pop_back()
    {
        base_type::decrement_size();
        decrement(&back_);

        (*back_).~value_type();
    }
};



}}