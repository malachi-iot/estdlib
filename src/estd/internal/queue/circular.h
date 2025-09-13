#pragma once

#include "circular/mutex.h"
#include "circular/policy.h"
#include "circular/base.h"

// Diagrams at https://drive.google.com/file/d/10WeFACvoEOZzTeRIDI_unXnSP5WJqY9f
// DEBT: Above link is clunky... make it more directly go to diagrams

// We are tuned for Single Producer Single Consumer (SPSC) model

namespace estd { namespace internal {


template <class Policy, class Enabled = void>
class circular_queue_base;


template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::bare>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

    static_assert(base_type::no_rollover == false, "Bare does not support no_rollover");

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
            return array_.size() - (front_ - back_);
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
    using base_type::no_rollover;

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
        // bump front first so that retrieval theoretically has less opportunity to see destructed
        // back
        increment(&front_);
        (*back).~value_type();
    }

    void rollunder(pointer front)
    {
        decrement(&back_);
        (*front).~value_type();
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

    template <class F, class Mutex = circular_mutex_noop>
    bool push_back_op(F&& f, Mutex mutex = {})
    {
        // back doesn't need a lock since we implicitly own it as the caller doing the push
        // (SPSC) - this means that MT simultaneous push to front and back is undefined

        pointer back = back_;

        if(type != queue_options::sentinel)
        {
            mutex.lock_front();
            mutex.lock_count();

            const bool full = !empty() && back == front_;
            if(full)
            {
                if(no_rollover)
                {
                    mutex.unlock_count();
                    mutex.unlock_front();
                    return false;
                }

                rollover(back);
            }
            else
                increment_size();

            mutex.unlock_count();
            mutex.unlock_front();
        }

        pointer dest = back;
        increment(&back);

        if(type == queue_options::sentinel)
        {
            mutex.lock_front();

            if(back == front_)
            {
                if(no_rollover)
                {
                    mutex.unlock_front();
                    return false;
                }

                rollover(back);
            }

            mutex.unlock_front();
        }

        f(dest);
        back_ = back;

        return true;
    }


    template <class Mutex = circular_mutex_noop>
    bool push_back(const_reference value, Mutex mutex = {})
    {
        return push_back_op([value](pointer back){ *back = value;}, std::forward<Mutex>(mutex));
    }

    template <class Mutex = circular_mutex_noop>
    pointer push_front_begin(Mutex mutex = {})
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


    template <class Mutex = circular_mutex_noop>
    void pop_front(Mutex mutex = {})
    {
        mutex.lock_front();
        mutex.lock_count();

        (*front_).~value_type();

        increment(&front_);
        decrement_size();

        mutex.unlock_count();
        mutex.unlock_front();
    }

    template <class Mutex = circular_mutex_noop>
    void pop_back(Mutex mutex = {})
    {
        mutex.lock_back();
        mutex.lock_count();

        decrement(&back_);
        decrement_size();       // 'flagged' mode seems to want this after decrement

        (*back_).~value_type();

        mutex.unlock_count();
        mutex.unlock_back();
    }

    ESTD_CPP_CONSTEXPR(14) void clear()
    {
        destruct();

        front_ = back_ = &array_[0];
    }
};



}}
