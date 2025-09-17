#pragma once

#include "circular/mutex.h"
#include "circular/policy.h"
#include "circular/base.h"
#include "circular/counter.h"
#include "circular/flagged.h"
#include "circular/sentinel.h"

// Diagrams at https://drive.google.com/file/d/10WeFACvoEOZzTeRIDI_unXnSP5WJqY9f
// DEBT: Above link is clunky... make it more directly go to diagrams

// We are tuned for Single Producer Single Consumer (SPSC) model

namespace estd { namespace internal {


template <class Policy>
class circular_queue_base<Policy, enable_if_t<Policy::type == queue_options::bare>> :
    public circular_queue_container_base<Policy>
{
    using base_type = circular_queue_container_base<Policy>;

    static_assert(base_type::no_rollover == false, "Bare does not support no_rollover");

protected:
    static bool ESTD_CPP_CONSTEVAL clear_size() { return{}; }
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
class circular_queue : public circular_queue_base<Policy>
{
    using base_type = circular_queue_base<Policy>;
    using typename base_type::container_policy;
    using typename base_type::container_type;

    // DEBT: Consider using allocator_traits::construct as per
    // https://eel.is/c++draft/container.requirements.general

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
        using base_type::iterator_base::bump;

    public:
        constexpr explicit iterator_base(
            circular_queue& parent,
            pointer current,
            size_t pos) :
            base_type::iterator_base(parent, current, pos)
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

    template <class Mutex = circular_mutex_noop>
    pointer push_back_begin(Mutex&& mutex = {})
    {
        // back doesn't need a lock since we implicitly own it as the caller doing the push
        // (SPSC) - this means that MT simultaneous push to front and back is undefined

        pointer back = back_;

        // DEBT: Euphemism for pure atomic mode.  Needs refinement
        if(!no_rollover) mutex.lock_front();

        if(type != queue_options::sentinel)
        {
            mutex.lock_count();

            const bool full = !empty() && back == front_;
            if(full)
            {
                if(no_rollover)
                {
                    mutex.unlock_count();
                    return nullptr;
                }

                rollover(back);
            }
            else
                increment_size();

            mutex.unlock_count();
        }

        pointer dest = back;
        increment(&back);

        if(type == queue_options::sentinel)
        {
            if(back == front_)
            {
                if(no_rollover) return nullptr;

                rollover(back);
            }
        }

        if(!no_rollover) mutex.unlock_front();

        back_ = back;

        return dest;
    }

    template <class F, class Mutex = circular_mutex_noop>
    pointer push_back_op(F&& f, Mutex&& mutex = {})
    {
        pointer dest = push_back_begin(std::forward<Mutex>(mutex));
        if(dest == nullptr) return nullptr;
        f(dest);
        return dest;
    }

    template <class Mutex = circular_mutex_noop>
    bool push_back(const_reference value, Mutex&& mutex = {})
    {
        return push_back_op([&value](pointer back)
        {
            new (back) value_type(value);
        }, std::forward<Mutex>(mutex));
    }

    template <class Mutex = circular_mutex_noop>
    bool push_back(value_type&& value, Mutex&& mutex = {})
    {
        return push_back_op([&value](pointer back)
        {
            new (back) value_type(std::move(value));
        }, std::forward<Mutex>(mutex));
    }

    template <class Mutex = circular_mutex_noop>
    pointer push_front_begin(Mutex&& mutex = {})
    {
        // front doesn't need a lock since we implicitly own it as the caller doing the push
        // (SPSC) - this means that MT simultaneous push to front and back is undefined

        pointer front = front_;

        // DEBT: Euphemism for pure atomic mode.  Needs refinement
        if(!no_rollover) mutex.lock_back();

        if(type != queue_options::sentinel)
        {
            mutex.lock_count();
            const bool full = !empty() && back_ == front;
            if(full)
            {
                if(no_rollover)
                {
                    mutex.unlock_count();
                    return nullptr;
                }

                rollunder(front);
            }
            else
                increment_size();
            mutex.unlock_count();
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

        if(!no_rollover) mutex.unlock_back();

        return front_ = front;
    }

    template <class F, class Mutex = circular_mutex_noop>
    pointer push_front_op(F&& f, Mutex&& mutex = {})
    {
        pointer dest = push_front_begin(std::forward<Mutex>(mutex));
        if(dest == nullptr) return nullptr;
        f(dest);
        return dest;
    }

    template <class Mutex = circular_mutex_noop>
    pointer push_front(const_reference value, Mutex&& mutex = {})
    {
        pointer dest = push_front_begin(std::forward<Mutex>(mutex));
        if(dest == nullptr) return nullptr;
        return new (dest) value_type(value);
    }


    template <class Mutex = circular_mutex_noop>
    pointer push_front(value_type&& value, Mutex&& mutex = {})
    {
        pointer dest = push_front_begin(std::forward<Mutex>(mutex));
        if(dest == nullptr) return nullptr;
        return new (dest) value_type(std::move(value));
    }

    template <class ...Args>
    pointer emplace_front(Args&&...args)
    {
        pointer dest = push_front_begin();
        if(dest == nullptr) return nullptr;
        return new (dest) value_type(std::forward<Args>(args)...);
    }

    // EXPERIMENTAL naming
    template <class Mutex, class ...Args>
    pointer emplace_front_mutex(Mutex&& mutex, Args&&...args)
    {
        pointer dest = push_front_begin(std::forward<Mutex>(mutex));
        if(dest == nullptr) return nullptr;
        return new (dest) value_type(std::forward<Args>(args)...);
    }

    template <class ...Args>
    bool emplace_back(Args&&...args)
    {
        return push_back_op([&args...](pointer back)
        {
            new (back) value_type(std::forward<Args>(args)...);
        });
    }

    // EXPERIMENTAL naming
    template <class Mutex, class ...Args>
    bool emplace_back_mutex(Mutex&& mutex, Args&&...args)
    {
        return push_back_op([&args...](pointer back)
        {
            new (back) value_type(std::forward<Args>(args)...);
        }, std::forward<Mutex>(mutex));
    }


    template <class Mutex = circular_mutex_noop>
    void pop_front(Mutex&& mutex = {})
    {
        mutex.lock_front();
        if(type != queue_options::sentinel) mutex.lock_count();

        (*front_).~value_type();

        increment(&front_);
        decrement_size();

        if(type != queue_options::sentinel) mutex.unlock_count();
        mutex.unlock_front();
    }

    template <class Mutex = circular_mutex_noop>
    void pop_back(Mutex mutex = {})
    {
        mutex.lock_back();
        if(type != queue_options::sentinel) mutex.lock_count();

        decrement(&back_);
        decrement_size();       // 'flagged' mode seems to want this after decrement

        (*back_).~value_type();

        if(type != queue_options::sentinel) mutex.unlock_count();
        mutex.unlock_back();
    }

    template <class Mutex = circular_mutex_noop>
    ESTD_CPP_CONSTEXPR(14) void clear(Mutex&& mutex = {})
    {
        // DEBT: Do we need to lock_count here?  If no, document why not
        mutex.lock_front();
        mutex.lock_back();
        destruct();

        front_ = back_ = &array_[0];
        base_type::clear_size();
        mutex.unlock_front();
        mutex.unlock_back();
    }
};



}}
