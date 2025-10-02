#pragma once

#include "circular/mutex.h"
#include "circular/policy.h"
#include "circular/base.h"
#include "circular/counter.h"
#include "circular/flagged.h"
#include "circular/packed.h"
#include "circular/sentinel.h"

// Diagrams at https://drive.google.com/file/d/10WeFACvoEOZzTeRIDI_unXnSP5WJqY9f
// DEBT: Above link is clunky... make it more directly go to diagrams

// We are tuned for Single Producer Single Consumer (SPSC) model

namespace estd { namespace internal {


// DEBT: Move him to his own spot
template <class Policy>
class circular_queue_impl<Policy, enable_if_t<Policy::type == queue_options::bare>> :
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

// FIX: Whoops, continued to misplace things under queue when it ought to be deque.
// circular_queue can skate by that.  But we need to fix it generally

template <class Policy>
class circular_queue : public circular_queue_impl<Policy>
{
    using base_type = circular_queue_impl<Policy>;
    using typename base_type::container_type;

    // DEBT: Consider using allocator_traits::construct as per
    // https://eel.is/c++draft/container.requirements.general

#if UNIT_TESTING
public:
#endif
    ESTD_CPP_STD_VALUE_TYPE(typename container_type::value_type)

    using base_type::front_;
    using base_type::back_;

    using base_type::hardened;
    using base_type::strict;

    using base_type::increment;
    using base_type::decrement;
    using base_type::increment_size;
    using base_type::decrement_size;
    using base_type::no_rollover;

    static constexpr bool is_trivial = Policy::is_trivial;

    template <bool forward>
    class iterator_base : public base_type::iterator
    {
        using base_type::iterator::bump;

    public:
        constexpr explicit iterator_base(
            circular_queue& parent,
            pointer current,
            size_t pos) :
            base_type::iterator(parent, current, pos)
        {

        }

        ESTD_CPP_CONSTEXPR(14) iterator_base& operator++()
        {
            bump(bool_constant<forward>{});
            return *this;
        }

        ESTD_CPP_CONSTEXPR(14) iterator_base operator++(int)
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

        ESTD_CPP_CONSTEXPR(14) iterator_base operator--(int)
        {
            iterator_base temp = *this;
            operator--();
            return temp;
        }
    };

    static constexpr bool destroy(pointer, true_type /* is_trivial */) { return {}; }
    static ESTD_CPP_CONSTEXPR(14) bool destroy(pointer v, false_type /* is_trivial */)
    {
        (*v).~value_type();
        return {};
    }

    static constexpr bool destroy(pointer v)
    {
        return destroy(v, bool_constant<is_trivial>{});
    }

    ESTD_CPP_CONSTEXPR(14) void rollover(pointer back)
    {
        // bump front first so that retrieval theoretically has less opportunity to see destructed
        // back
        increment(&front_);
        destroy(back, bool_constant<is_trivial>{});
    }

    ESTD_CPP_CONSTEXPR(14) void rollunder(pointer front)
    {
        decrement(&back_);
        destroy(front, bool_constant<is_trivial>{});
    }

    static constexpr bool destroy(true_type /* is_trivial */) { return {}; }

    ESTD_CPP_CONSTEXPR(14) void destroy(false_type /* is_trivial */)
    {
        for(iterator i = begin(); i != end(); ++i)
            i->~value_type();
    }

public:
    using base_type::type;
    using base_type::empty;
    using base_type::size;

    constexpr circular_queue() = default;
    constexpr circular_queue(const circular_queue&) = default;
    // 17SEP25 No move needed at this time - prepared for future more exotic
    // allocated containers.
    constexpr circular_queue(circular_queue&&) = default;

    ESTD_CPP_DEFAULT_ASSIGNMENT(circular_queue)

    template <class ...Args>
    constexpr explicit circular_queue(in_place_t, Args&&...args) :
        base_type(in_place_t{}, std::forward<Args>(args)...)
    {}

    ~circular_queue() { destroy(bool_constant<is_trivial>{}); }

    using iterator = iterator_base<true>;
    using reverse_iterator = iterator_base<false>;
    using const_iterator = const iterator;
    using const_reverse_iterator = const reverse_iterator;

    // DEBT: Just for compatibility with legacy::deque
    using forward_iterator = iterator;

    iterator begin()
    {
        return iterator{*this, front_, 0};
    }

    constexpr const_iterator begin() const
    {
        return const_iterator{*this, front_, 0};
    }

    constexpr const_iterator cbegin() const
    {
        return const_iterator{*this, front_, 0};
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

    constexpr const_iterator cend() const
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

    template <class F,class Mutex = circular_mutex_noop>
    pointer push_back_op(F&& f, Mutex&& mutex = {})
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

        // initializes *back_ just before revealing that it's
        // available
        f(dest);

        back_ = back;

        return dest;
    }

    template <class Mutex = circular_mutex_noop>
    pointer push_back(const_reference value, Mutex&& mutex = {})
    {
        return push_back_op([&value](pointer back)
        {
            new (back) value_type(value);
        }, std::forward<Mutex>(mutex));
    }

    template <class Mutex = circular_mutex_noop>
    pointer push_back(value_type&& value, Mutex&& mutex = {})
    {
        return push_back_op([&value](pointer back)
        {
            new (back) value_type(std::move(value));
        }, std::forward<Mutex>(mutex));
    }

    template <class F, class Mutex = circular_mutex_noop>
    pointer push_front_op(F&& f, Mutex&& mutex = {})
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

        // Initialize *front before revealing it as available
        f(front);

        return front_ = front;
    }

    template <class Mutex = circular_mutex_noop>
    pointer push_front(const_reference value, Mutex&& mutex = {})
    {
        return push_front_op([&value](pointer front)
        {
            return new (front) value_type(value);
        }, std::forward<Mutex>(mutex));
    }


    template <class Mutex = circular_mutex_noop>
    pointer push_front(value_type&& value, Mutex&& mutex = {})
    {
        return push_front_op([&value](pointer front)
        {
            return new (front) value_type(std::move(value));
        }, std::forward<Mutex>(mutex));
    }

    template <class ...Args>
    pointer emplace_front(Args&&...args)
    {
        return push_front_op([&args...](pointer front)
        {
            return new (front) value_type(std::forward<Args>(args)...);
        });
    }

    // EXPERIMENTAL naming
    template <class Mutex, class ...Args>
    pointer emplace_front_mutex(Mutex&& mutex, Args&&...args)
    {
        return push_front_op([&args...](pointer front)
        {
            return new (front) value_type(std::forward<Args>(args)...);
        }, std::forward<Mutex>(mutex));
    }

    template <class ...Args>
    pointer emplace_back(Args&&...args)
    {
        return push_back_op([&args...](pointer back)
        {
            new (back) value_type(std::forward<Args>(args)...);
        });
    }

    // EXPERIMENTAL naming
    template <class Mutex, class ...Args>
    pointer emplace_back_mutex(Mutex&& mutex, Args&&...args)
    {
        return push_back_op([&args...](pointer back)
        {
            new (back) value_type(std::forward<Args>(args)...);
        }, std::forward<Mutex>(mutex));
    }


    template <class Mutex = circular_mutex_noop>
    bool pop_front(Mutex&& mutex = {})
    {
        ESTD_CPP_IF_CONSTEXPR(strict && empty()) return false;
        ESTD_CPP_IF_CONSTEXPR(hardened) assert(!empty());

        mutex.lock_front();
        if(type != queue_options::sentinel) mutex.lock_count();

        destroy(front_);

        increment(&front_);
        decrement_size();

        if(type != queue_options::sentinel) mutex.unlock_count();
        mutex.unlock_front();
        return true;
    }

    template <class Mutex = circular_mutex_noop>
    bool pop_back(Mutex mutex = {})
    {
        ESTD_CPP_IF_CONSTEXPR(strict && empty()) return false;
        ESTD_CPP_IF_CONSTEXPR(hardened) assert(!empty());

        mutex.lock_back();
        if(type != queue_options::sentinel) mutex.lock_count();

        decrement(&back_);
        decrement_size();       // 'flagged' mode seems to want this after decrement

        destroy(back_);

        if(type != queue_options::sentinel) mutex.unlock_count();
        mutex.unlock_back();
        return true;
    }

    template <class Mutex = circular_mutex_noop>
    ESTD_CPP_CONSTEXPR(14) void clear(Mutex&& mutex = {})
    {
        // DEBT: Do we need to lock_count here?  If no, document why not
        mutex.lock_front();
        mutex.lock_back();
        destroy(bool_constant<is_trivial>{});

        base_type::clear();
        base_type::clear_size();
        mutex.unlock_front();
        mutex.unlock_back();
    }
};



}}
