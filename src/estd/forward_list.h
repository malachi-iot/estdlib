#pragma once

#include "../platform.h"

namespace estd {

template <class TNode> struct node_traits;

namespace experimental {
// Make list and forward list conform to, but not necessarily depend on, these
// signatures (C++ concepts would be nice here)

class forward_node_base
{
    forward_node_base* m_next;

public:
    forward_node_base() : m_next(NULLPTR) {}

    const forward_node_base* next() const { return m_next; }
    void next(forward_node_base* set_to) { m_next = set_to; }
};

class reverse_node_base
{
    reverse_node_base* m_prev;

public:
    reverse_node_base() : m_prev(NULLPTR) {}

    const reverse_node_base* prev() const { return m_prev; }
    void prev(reverse_node_base* set_to) { m_prev = set_to; }
};

template <class T>
class forward_node :
    public forward_node_base
{
    T m_value;

public:
    T& value() { return m_value; }

    const forward_node<T>* next() const
    {
        return (forward_node<T>*)forward_node_base::next();
    }

    void next(forward_node<T>* set_to)
    {
        forward_node_base::next(set_to);
    }
};

template <class T>
class double_node :
        public forward_node_base,
        public reverse_node_base
{
    T m_value;

public:
    T& value() { return m_value; }
};



}

/*
template <class TForwardNodeValue, class TFowardNode = experimental::forward_node<TForwardNodeValue > >
struct node_traits
{
    typedef TForwardNodeValue value_type;
    typedef TFowardNode node_type;

    template <class TList>
    static node_type& front(const TList& list) { return list.front(); }

    static const node_type* get_next(const node_type& node) { return node.next(); }
    static void set_next(node_type& node, node_type* set_to) { node.next(set_to); }
    static value_type& value(node_type& node) { return node.value(); }
};
*/

// this is where node and value are combined
template <class TForwardNodeValue>
struct node_traits
{
    typedef TForwardNodeValue value_type;
    typedef value_type node_type;

    template <class TList>
    static node_type& front(const TList& list) { return list.front(); }

    static const node_type* get_next(const node_type& node) { return node.next(); }
    static void set_next(node_type& node, node_type* set_to) { node.next(set_to); }
    static value_type& value(node_type& node) { return node.value(); }
};


template <>
struct node_traits<experimental::forward_node_base>
{
    typedef experimental::forward_node_base node_type;

    static const node_type* get_next(const node_type& node) { return node.next(); }
    static void set_next(node_type& node, node_type* set_to) { node.next(set_to); }
};


template <class T>
class forward_list
{
    T* m_front;

public:
    forward_list() : m_front(NULLPTR) {}

    typedef T value_type;
    typedef value_type& reference;
    typedef node_traits<T> node_traits_t;

    reference front() { return *m_front; }

    void push_front(T& value)
    {
        if(m_front != NULLPTR)
            node_traits_t::set_next(value, m_front);

        m_front = &value;
    }
};

}
