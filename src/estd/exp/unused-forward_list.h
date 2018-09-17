#ifdef UNUSED
// yanked directly in from util.embedded
// like it, but I don't like how node_traits directly reflect and require a list_type
namespace exp1 {


template <class TNode, class TAllocatorTraits = allocator_traits<Allocator<TNode>>>
class node_pointer
{
protected:
    typedef TNode node_type;
    typedef TAllocatorTraits node_allocator_traits;
    typedef node_pointer<node_type> this_t;
    typedef typename node_allocator_traits::pointer _node_pointer;

    _node_pointer current;

    node_pointer(node_type* current) : current(current) {}

public:
    _node_pointer getCurrent() const { return current; }

    bool operator==(const this_t& rhs)
    {
        return current == rhs.getCurrent();
    }

    bool operator!=(const this_t& rhs)
    {
        return current != rhs.getCurrent();
    }
};





template <class T, class TNode,
        class TNodeAllocatorTraits = allocator_traits<Allocator<TNode>>>
struct node_allocator
{
public:
    typedef T value_type;
    typedef TNode node_type;
    typedef TNodeAllocatorTraits node_allocator_traits;
    typedef allocator_traits<Allocator<value_type>> value_allocator_traits;

    typedef typename node_allocator_traits::pointer node_pointer;
    typedef typename value_allocator_traits::pointer value_pointer;

    typedef typename value_allocator_traits::const_void_pointer const_void_pointer;

    node_pointer allocate(value_pointer reference)
    {
        return reference;
    }

    void deallocate(node_pointer node)
    {
        // Maybe setting node->next to null here would be prudent?
    }

    // hint helps us track down what the associated value is
    // perhaps the node * is not in our control
    static value_pointer get_associated_value(node_pointer node, const_void_pointer hint)
    {
        return static_cast<value_pointer>(node);
    }
};


struct OutputIterator {};


template <class TNodeAllocator, const void* hint = nullptr,
        class TBase = node_pointer<typename TNodeAllocator::node_type>>
struct InputIterator : public TBase
{
    typedef typename TNodeAllocator::value_type value_type;
    typedef typename TNodeAllocator::node_type node_type;
    typedef TBase base_t;

    InputIterator(node_type* node) : base_t(node) {}


    // FIX: doing for(auto i : list) seems to do a *copy* operation
    // for(value_type& i : list) is required to get a reference.  Check to see if this is
    // proper behavior
    value_type& operator*()
    {
        return *TNodeAllocator::get_associated_value(base_t::getCurrent(), hint);
    }
};



template <class TNodeAllocator, class TBase = node_pointer<typename TNodeAllocator::node_type>>
struct ForwardIterator : public InputIterator<TNodeAllocator, nullptr, TBase>
{
    typedef typename TNodeAllocator::node_type   node_type;
    typedef typename TNodeAllocator::value_type  value_type;
    typedef InputIterator<TNodeAllocator, nullptr, TBase> base_t;

    ForwardIterator(const ForwardIterator& source) :
            base_t(source)
    {
    }

    ForwardIterator(node_type* node) :
            base_t(node)
    {
    }


    ForwardIterator& operator++()
    {
        base_t::current = node_traits<node_type>::get_next(base_t::current);
        return *this;
    }

    // postfix version
    ForwardIterator operator++(int)
    {
        ForwardIterator temp(*this);
        operator++();
        return temp;
    }
};


template <class TNodeAllocator, class TBase = node_pointer<typename TNodeAllocator::node_type>>
struct BidirectionalIterator : public ForwardIterator<TNodeAllocator, TBase>
{
    typedef typename TNodeAllocator::node_type   node_type;
    typedef typename TNodeAllocator::value_type  value_type;
    typedef ForwardIterator<TNodeAllocator, TBase> base_t;

    BidirectionalIterator(const BidirectionalIterator& source) :
            base_t(source)
    {
    }

    BidirectionalIterator(node_type* node) :
            base_t(node)
    {
    }


    BidirectionalIterator& operator--()
    {
        base_t::current = node_traits<node_type>::get_prev(base_t::current);
        return *this;
    }

    // postfix version
    BidirectionalIterator operator--(int)
    {
        BidirectionalIterator temp(*this);
        operator--();
        return temp;
    }
};


// TODO: move these to type_traits.h
template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };


template <class TNodeAllocator, class TIterator>
class list_base
{
public:
    typedef typename TNodeAllocator::node_type node_type;
    typedef node_traits<node_type> node_traits_t;
    typedef typename node_traits_t::list_type list_type;
    typedef typename TNodeAllocator::value_type value_type;
    typedef typename TNodeAllocator::node_pointer node_pointer;
    typedef typename TNodeAllocator::value_pointer value_pointer;
    typedef list_base<TNodeAllocator, TIterator> this_t;

    typedef TIterator        iterator;
    typedef const iterator   const_iterator;

    typedef value_type&     reference;

protected:
    list_type list;
    TNodeAllocator node_allocator;

    TNodeAllocator& get_node_allocator()
    {
        return node_allocator;
    }

    inline node_pointer get_head() const
    {
        return node_traits_t::get_head(&list);
    }

    node_pointer _pop_front()
    {
        node_pointer node = get_head();
        list.experimental_set_head(node_traits_t::get_next(node));
        return node;
    }

public:
    bool empty() const { return get_head() == nullptr; }

    iterator begin() const { return iterator(get_head()); }
    iterator end() const { return iterator(nullptr); }

    reference front() const { return *begin(); }

    // not a const like in standard because we expect to actually modify
    // the prev/next parts of value
    void push_front(value_type& value)
    {
        node_pointer node = get_node_allocator().allocate(&value);

        list.insertAtBeginning(node);
    }


    void pop_front()
    {
        node_pointer node = _pop_front();
        get_node_allocator().deallocate(node);
    }

    friend struct helper;

    template <class T>
    struct helper
    {
        template <class Q = T>
        static typename enable_if<Q::has_tail() == true>::type
        handle_tail(this_t* t)
        {
            node_traits_t::get_tail(&t->list);
        }


        template <class Q = T>
        static typename enable_if<Q::has_tail() == false>::type
        handle_tail(this_t* t)
        {
        }
    };



    // TODO: determine if this works properly for double-linked as well
    // FIX: need to add "tail" awareness probably with SFINAE
    iterator erase_after(const_iterator pos)
    {
        node_pointer pos_node = pos.getCurrent();
        node_pointer node_to_erase = node_traits_t::get_next(pos_node);

        node_traits_t::remove_next(pos_node);

        // FIX: if node_to_erase == tail, we need to bump tail down by one,
        // effectively a pop_back
        helper<node_traits_t>::handle_tail(this);

        get_node_allocator().deallocate(node_to_erase);
        return iterator(node_traits_t::get_next(pos_node));
    }
};

template <class T, class TNodeAllocator = node_allocator<T, experimental::forward_node_base>>
class forward_list :
        public list_base<TNodeAllocator, ForwardIterator<TNodeAllocator>>
{
    typedef T value_type;
    typedef value_type &reference;
    typedef const value_type &const_reference;

    typedef list_base<TNodeAllocator, ForwardIterator<TNodeAllocator>> base_t;
    typedef typename base_t::node_traits_t node_traits_t;

    typedef typename base_t::node_type node_type;
    typedef typename base_t::node_pointer node_pointer;

    TNodeAllocator &get_node_allocator()
    {
        return base_t::node_allocator;
    }

public:
    typedef ForwardIterator<TNodeAllocator> iterator;
    typedef const iterator const_iterator;


    // non-standard:
    // removes/deallocate node at pos and splices in value
    const_iterator replace_after(const_iterator pos, value_type& value)
    {
        auto node_allocator = get_node_allocator();

        node_type* pos_node = pos.getCurrent();
        // old 'next' node prep it for erase
        node_type* node_to_erase = node_traits_t::get_next(pos_node);
        // new 'next' node allocate node portion, if necessary
        node_type* new_node = node_allocator.allocate(&value);

        // TODO: set pos_node->next to be &value
        // inset new 'next' node after current node and before old 'next''s next
        // node
        new_node->insertBetween(pos_node, node_to_erase->getNext());

        //
        node_allocator.deallocate(node_to_erase);

        return pos;
    }


    // Non-standard
    void replace_front(value_type& value)
    {
        auto node_allocator = get_node_allocator();

        node_type* front_node = base_t::get_head();

        node_type* new_front_node = node_allocator.allocate(value);

        new_front_node->insertBetween(nullptr, front_node->getNext());

        base_t::list.experimental_set_head(new_front_node);

    }


    iterator insert_after(const_iterator pos, value_type& value)
    {
        node_type* pos_node = pos.getCurrent();
        node_type* node = get_node_allocator().allocate(&value);

        // FIX: insertBetween is overcompliated, the insert_after is cleaner and better
        // (the getNext() is always the value used, so why bother making it an explicit param)
        base_t::list.insertBetween(pos_node, pos_node->getNext(), node);

        return iterator(node);
    }

    // Non-standard, eliminate this call in favor of more manual pop_front/etc
    void remove(reference r)
    {
        base_t::list.remove(&r);
    }

};



template <class T, class TNodeAllocator = node_allocator<T, experimental::double_node_base>>
class list :
        public list_base<TNodeAllocator, BidirectionalIterator<TNodeAllocator>>
{
    typedef T                   value_type;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;

    typedef list_base<TNodeAllocator, BidirectionalIterator<TNodeAllocator>> base_t;
    typedef typename base_t::node_traits_t  node_traits_t;

    typedef typename base_t::node_type      node_type;
    typedef typename base_t::node_pointer   node_pointer;

    TNodeAllocator& get_node_allocator()
    {
        return base_t::node_allocator;
    }

public:
    typedef BidirectionalIterator<TNodeAllocator>         iterator;
    typedef const iterator   const_iterator;

    reference back()
    {
        node_pointer ptr = base_t::list.experimental_get_tail();
        reference value = get_node_allocator().get_associated_value(ptr);
        return value;
    }

    void pop_back()
    {
        // If not empty
        if(!base_t::empty())
        {
            // grab very last entry
            node_pointer tail = base_t::list.experimental_get_tail();

            // then grab entry just before last entry
            node_pointer prev = node_traits_t::get_prev(tail);

            // FIX: removeNext does more work than we need, it
            // actually does prev->next = prev->next->next
            prev->removeNext();

            // if necessary, deallocate memory for tail node
            get_node_allocator().deallocate(tail);

            // now set tail to previous entry
            base_t::list.experimental_set_tail(prev);
        }
    }
};
}
#endif