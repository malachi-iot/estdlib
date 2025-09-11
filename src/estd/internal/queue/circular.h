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
    // EXPERIMENTAL - inhibit would-be exceptions/invalid condition checks
    // similar to c++26 contracts as per
    // https://en.cppreference.com/w/cpp/container/deque/pop_back.html
    // https://en.cppreference.com/w/cpp/language/contracts.html
    strict      = 0x0040,

    default_opt = flagged
};

ESTD_FLAGS(queue_options)


template <class T, queue_options o, class Nullable = nullable_traits<T>>
struct circular_policy
{
    static constexpr queue_options type = o & queue_options::mask;
    static constexpr bool atomic = is_set(o & queue_options::atomic);
    constexpr static bool is_trivial = is_set(o & queue_options::trivial) || is_integral<T>::value
#if FEATURE_ESTD_IS_TRIVIAL
        || estd::is_trivial<T>::value
#endif
        ;

    using nullable = Nullable;
    static constexpr queue_options options = o;
};

template <class T, size_t N, queue_options o>
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

template <class T, size_t N, queue_options o>
struct span_circular_policy : circular_policy<T, o>
{
    using container_type = estd::span<T, N>;
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

template <class Policy, class Enabled = void>
class circular_queue_base;


template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::bare>> :
    public circular_queue_container_base<Policy>
{
protected:
};



template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::sentinel>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

protected:
    using base_type::back_;
    using base_type::front_;
    using base_type::array_;

    constexpr circular_queue_base() = default;

    template <class ...Args>
    constexpr explicit circular_queue_base(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

public:
    using typename base_type::size_type;

    constexpr bool empty() const
    {
        return front_ == back_;
    }

    size_type size() const
    {
        if(front_ > back_)
            return array_.size() - (front_ - back_);
        else
            return back_ - front_;
    }
};


template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::flagged>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

protected:
    using base_type::atomic;
    using base_type::array_;
    using base_type::front_;
    using base_type::back_;

    using iterator_base = typename base_type::pos_iterator_base;

#if FEATURE_STD_ATOMIC
    using bool_type = conditional_t<atomic, std::atomic_flag, bool>;
#else
    using bool_type = bool;
#endif

    bool_type empty_{true};

    ESTD_CPP_CONSTEXPR(14) void increment_size()
    {
        empty_ = false;
    }

    ESTD_CPP_CONSTEXPR(14) void decrement_size()
    {
        empty_ = back_ == front_;
    }

    constexpr circular_queue_base() = default;

    template <class ...Args>
    constexpr explicit circular_queue_base(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

public:
    constexpr bool empty() const { return empty_; }
    using typename base_type::size_type;

    size_type size() const
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
    using base_type = circular_queue_container_base<Policy>;

protected:
    using base_type::atomic;

    using iterator_base = typename base_type::pos_iterator_base;

#if FEATURE_STD_ATOMIC
    using counter_type = conditional_t<atomic, std::atomic<unsigned>, unsigned>;
#else
    using counter_type = unsigned;
#endif

    counter_type size_{};

    ESTD_CPP_CONSTEXPR(14) void decrement_size()
    {
        --size_;
    }

    ESTD_CPP_CONSTEXPR(14) void increment_size()
    {
        ++size_;
    }

    constexpr circular_queue_base() = default;

    template <class ...Args>
    constexpr explicit circular_queue_base(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

public:
    using typename base_type::size_type;

    constexpr bool empty() const { return size_ == 0; }
    constexpr size_type size() const { return size_; }
};


template <class Policy>
class circular_queue : public circular_queue_base<Policy>
{
    using base_type = circular_queue_base<Policy>;
    using typename base_type::container_policy;
    using typename base_type::container_type;

#if UNIT_TESTING
public:
#endif
    ESTD_CPP_STD_VALUE_TYPE(typename container_type::value_type)

    using base_type::array_;
    using base_type::front_;
    using base_type::back_;

    using base_type::increment;
    using base_type::decrement;
    using base_type::increment_size;
    using base_type::decrement_size;

    static constexpr bool no_rollover = is_set(Policy::options & queue_options::no_rollover);
    static constexpr bool strict = is_set(Policy::options & queue_options::strict);
    static constexpr bool is_trivial = Policy::is_trivial;

    template <bool forward>
    class iterator_base : public base_type::iterator_base
    {
        circular_queue& parent_;
        pointer current_;

        // In reverse mode, pos is +1 offset so as to not need signed for -1 rend position
        // DEBT: Sentinel flavor does not need pos_ - flagged/counter version does, optimize that
        unsigned pos_;

        void bump(true_type)
        {
            base_type::iterator_base::bump_up();

            ++pos_;
            parent_.increment(&current_);
        }

        void bump(false_type)
        {
            base_type::iterator_base::bump_down();

            --pos_;
            parent_.decrement(&current_);
        }

    public:
        constexpr explicit iterator_base(
            circular_queue& parent,
            pointer current,
            size_t pos) :
            parent_{parent},
            current_{current},
            pos_{static_cast<unsigned>(pos)}
        {

        }

        ESTD_CPP_CONSTEXPR(14) iterator_base& operator++()
        {
            bump(bool_constant<forward>{});
            return *this;
        }

        iterator_base operator++(int)
        {
            iterator_base temp = *this;
            operator++();
            return temp;
        }

        ESTD_CPP_CONSTEXPR(14) iterator_base& operator--()
        {
            bump(bool_constant<!forward>{});
            return *this;
        }

        iterator_base operator--(int)
        {
            iterator_base temp = *this;
            operator--();
            return temp;
        }

        reference operator*() { return *current_; }
        constexpr const_reference operator*() const { return *current_; }

        pointer operator->() { return current_; }
        constexpr const_pointer operator->() const { return current_; }

        constexpr bool operator==(const iterator_base& other) const
        {
            return pos_ == other.pos_;
            //return current_ == other.current_;
        }

        constexpr bool operator!=(const iterator_base& other) const
        {
            return pos_ != other.pos_;
            //return current_ != other.current_;
        }
    };

    void rollover(pointer back)
    {
        (*back).~value_type();
        increment(&front_);
    }

    void rollunder(pointer front)
    {
        (*front).~value_type();
        decrement(&back_);
    }

    ESTD_CPP_CONSTEXPR(14) void destruct()
    {
        if(is_trivial == false)
        {
            for(iterator i = begin(); i != end(); ++i)
                i->~value_type();
        }
    }

public:
    using base_type::type;
    using base_type::empty;
    using base_type::size;

    // Args... flavor handles this mostly, but nice to have for debugging simplicity
    // and the occasional implicit construction.
    constexpr circular_queue() = default;

    template <class ...Args>
    constexpr explicit circular_queue(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {}

    ~circular_queue() { destruct(); }

    using iterator = iterator_base<true>;
    using reverse_iterator = iterator_base<false>;
    using const_iterator = const iterator;
    using const_reverse_iterator = const reverse_iterator;

    iterator begin()
    {
        return iterator{*this, front_, 0};
    }

    reverse_iterator rbegin()
    {
        pointer back = back_;
        decrement(&back);
        return reverse_iterator{*this, back, size()};
    }

    iterator end()
    {
        return iterator{*this, back_, size()};
    }

    constexpr const_iterator end() const
    {
        return const_iterator{*this, back_, size()};
    }

    reverse_iterator rend()
    {
        pointer front = front_;
        decrement(&front);
        return reverse_iterator{*this, front, 0};
    }


    reference back()
    {
        pointer i = back_;

        decrement(&i);

        return *i;
    }

    template <class F>
    bool push_back_op(F&& f)
    {
        pointer back = back_;

        if(type != queue_options::sentinel)
        {
            const bool full = !empty() && back == front_;
            if(full)
            {
                if(no_rollover) return false;

                rollover(back);
            }
            else
                increment_size();
        }

        pointer dest = back;
        increment(&back);

        if(type == queue_options::sentinel)
        {
            if(back == front_)
            {
                if(no_rollover) return false;

                rollover(back);
            }
            else
                increment_size();
        }

        f(dest);
        back_ = back;

        return true;
    }


    bool push_back(const_reference value)
    {
        return push_back_op([value](pointer back){ *back = value;});
    }

    pointer push_front_begin()
    {
        pointer front = front_;

        if(type != queue_options::sentinel)
        {
            // UNTESTED
            const bool full = !empty() && back_ == front;
            if(full)
            {
                if(no_rollover) return nullptr;

                rollunder(front);
            }
            else
                increment_size();
        }

        decrement(&front);

        if(type == queue_options::sentinel)
        {
            if(front == back_)
            {
                if(no_rollover) return nullptr;

                rollunder(front);
            }
            else
                increment_size();
        }

        return front_ = front;
    }

    template <class F>
    bool push_front_op(F&& f)
    {
        pointer dest = push_front_begin();
        if(dest == nullptr) return false;
        f(dest);
        return true;
    }

    void push_front(const_reference value)
    {
        *push_front_begin() = value;
    }

    template <class ...Args>
    void emplace_front(Args&&...args)
    {
        new (push_front_begin()) value_type(std::forward<Args>(args)...);
    }


    template <class ...Args>
    bool emplace_back(Args&&...args)
    {
        return push_back_op([&args...](pointer back)
        {
            new (back) value_type(std::forward<Args>(args)...);
        });
    }


    void pop_front()
    {
        (*front_).~value_type();

        increment(&front_);
        decrement_size();
    }

    void pop_back()
    {
        decrement(&back_);
        decrement_size();       // 'flagged' mode seems to want this after decrement

        (*back_).~value_type();
    }

    ESTD_CPP_CONSTEXPR(14) void clear()
    {
        destruct();

        front_ = back_ = &array_[0];
    }
};



}}
