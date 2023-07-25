// aligned storage in general is deprecated, and for our purposes `uninitialized_array`
// fills that utility gap

namespace experimental {

//#ifdef FEATURE_CPP_ALIGN  // -- disabled during dev/visualdsp development

// remember, aligned_storage means uninitialized
// no constructors because the whole point is to leave this uninitialized
template <class T, size_t array_len>
struct aligned_storage_array
{
#ifdef FEATURE_CPP_ALIGN
    typedef typename estd::aligned_storage<sizeof(T), alignof (T)>::type item_type;
#else
    typedef typename estd::aligned_storage<sizeof(T), __alignof__ (T)>::type item_type;
#endif
    typedef T value_type;
    typedef value_type& reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    // FIX: iterator cannot be a pointer, because ++ or -- on an iterator may require
    // skipping more bytes than the native type would reflect
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef size_t size_type;

    struct _iterator_type
    {
    private:
        item_type* current;

    public:
        _iterator_type& operator++()
        {
            current++;
            return *this;
        }
    };

private:
    item_type  array[array_len];

public:
    // FIX: data() calls probably cannot exist unless alignment is already perfectly
    // matched with default aligntment
    pointer data()
    {
        return reinterpret_cast<pointer>(array);
    }

    const_pointer data() const
    {
        return reinterpret_cast<const_pointer>(array);
    }

    operator pointer()
    {
        return data();
    }

    reference operator[](size_t i)
    {
        item_type& v = array[i];

        return reinterpret_cast<reference>(v);
    }

    iterator begin() { return data(); }
    iterator end() { return data() + array_len; }

    const_iterator begin() const { return data(); }
    const_iterator end() const { return data() + array_len; }
};
//#endif
}
