#pragma once

namespace estd {

namespace experimental {

// Make list and forward list conform to, but not necessarily depend on, these
// signatures (C++ concepts would be nice here)
// TODO: fixup these names and move them out of experimental.  Remember, although
// all our node_traits and friends expect these signatures, you could roll your
// own node traits and use different signatures along the way

// TNode might be a handle instead of a direct pointer.  Perhaps our own kind of
// "fancy pointer"
template <class TNode>
class forward_node_base_base
{
protected:

    TNode m_next;

    forward_node_base_base(TNode initial_value) : m_next(initial_value) {}

public:
    typedef TNode node_t;

    node_t next() const { return m_next; }

    void next(node_t set_to) { m_next = set_to; }
};

class forward_node_base : public forward_node_base_base<forward_node_base*>
{
protected:
    typedef forward_node_base_base<forward_node_base*> base_t;

public:
    forward_node_base() : base_t(NULLPTR) {}
};

class reverse_node_base
{
    reverse_node_base* m_prev;

public:
    reverse_node_base() : m_prev(NULLPTR) {}

    reverse_node_base* prev() const { return m_prev; }

    void prev(reverse_node_base* set_to) { m_prev = set_to; }
};


class double_node_base :
        public forward_node_base,
        public reverse_node_base
{};

template<class T>
class forward_node :
        public forward_node_base {
    T m_value;

public:
    T& value() { return m_value; }

    const T& value() const { return m_value; }

    forward_node<T> *next() const {
        return (forward_node<T> *) forward_node_base::next();
    }

    void next(forward_node<T> *set_to) {
        forward_node_base::next(set_to);
    }

    bool operator==(const forward_node& compare_to) const
    {
        return m_value == compare_to.value();
    }
};

template<class T>
class double_node :
        public forward_node_base,
        public reverse_node_base {
    T m_value;

public:
    T &value() { return m_value; }
};

}

}
