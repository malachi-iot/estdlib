#pragma once

// In response to https://github.com/malachi-iot/estdlib/issues/11
// Undecided if we want to explicitly state forward_list here or
// if we hang it off traits.  I'd prefer to hang it off traits,
// but keeping it simpler to start with

namespace estd { namespace internal { namespace list {

// If we want an allocator (non intrusive), it will come along as part of traits
// though we might need an Impl for stateful allocators
template <class T>
struct intrusive_traits
{
    // Get next ptr
    static T* next(const T* node)
    {
        return node->next();
    }

    // Re-assign next ptr
    static void next(T* node, T* v)
    {
        node->next(v);
    }
};

template <class T, class Traits = intrusive_traits<T> >
struct intrusive_iterator
{

};

struct null_iterator {};

// without tail
template <class T, class Traits = intrusive_traits<T> >
class intrusive_forward
{
    ESTD_CPP_STD_VALUE_TYPE(T)

    pointer head_;

public:
    ESTD_CPP_CONSTEXPR_RET intrusive_forward() :
        head_(NULLPTR)
    {}

    class iterator
    {
        pointer current_;

    public:
        ESTD_CPP_CONSTEXPR_RET iterator(pointer start) :
            current_(start)
        {}

        ESTD_CPP_CONSTEXPR_RET iterator(null_iterator) :
            current_(nullptr)
        {}

        iterator& operator++()
        {
            current_ = Traits::next(current_);
            return *this;
        }

        iterator operator++(int) const
        {
            iterator temp(current_);
            ++*this;
            return temp;
        }

        iterator& operator--()
        {
            current_ = Traits::next(current_);
            return *this;
        }

        bool operator==(null_iterator) const
        {
            return current_ == nullptr;
        }

        bool operator!=(null_iterator) const
        {
            return current_ != nullptr;
        }

        bool operator==(const iterator& compare_to) const
        {
            return current_ == compare_to.current_;
        }
    };

    ESTD_CPP_CONSTEXPR_RET bool empty() const
    {
        return head_ == nullptr;
    }

    // TODO: clear() for intrusive takes on a different nature,
    // since we don't own the values per se.  Needs more attention
    void clear()
    {
        head_ = nullptr;
    }

    using const_iterator = const iterator;

    iterator begin()
    {
        return iterator(head_);
    }

    reference front()
    {
        return *head_;
    }

    ESTD_CPP_CONSTEXPR_RET null_iterator end() const { return null_iterator(); }

    iterator insert_after(const_iterator pos, reference value)
    {
        iterator i(pos);

        for(; i != end(); ++i)
        {

        }

        return i;
    }

    void push_front(reference value)
    {
        Traits::next(&value, head_);
        head_ = &value;
    }

    void pop_front()
    {
        head_ = Traits::next(head_);
    }
};

}}}