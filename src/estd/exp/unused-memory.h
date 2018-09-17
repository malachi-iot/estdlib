namespace internal {

template <class T>
struct shared_ptr_control_block
{
    T* shared;
    // seems very much like we'd want a polymorphic (or similar) allocator here
    union
    {
        struct
        {
            uint16_t shared_count : 7;
            uint16_t weak_count : 7;
            // indicates during inline mode if this control block is active
            // think of it as the 'master' active bit, even though subordinates
            // are independently active or not
            uint16_t is_active : 1;
            uint16_t is_inline : 1;
        };

        uint16_t raw;
    };
};

}

namespace experimental {

// TODO: use a memory pool to allocate a shared_ptr_control_block
template <class T, class TDeleter = void>
class shared_ptr
{
    T* stored;

    typedef estd::internal::shared_ptr_control_block<T> control_type;

    // since inline is always bigger than control_ptr, we're going to presume
    // is_inline is intact always
    union
    {
        control_type* control_ptr;
        // expanding size of shared_ptr from std a bit, but we avoid allocation.
        // also beware that the weak_count and shared_count only go to 127
        control_type control_inline;
    };

    control_type* control() const
    {
        if(control_inline.is_inline)
            return control_inline.is_active ? &control_inline : NULLPTR;
        else
            return control_ptr;
    }



public:
    typedef typename estd::remove_extent<T>::type element_type;

    template <class Y>
    explicit shared_ptr(Y* ptr) :
        stored(ptr)
    {
        control_inline.raw = 0;
        control_inline.is_inline = true;
        control_inline.is_active = true;
        control()->shared = ptr;
    }

    template <class Y>
    shared_ptr(const shared_ptr<Y>& r) noexcept
    {
        control_inline.raw = 0; // is_inline = false
        control_ptr = r.control();
    }

    element_type* get() const noexcept { return stored; }

    long use_count() const noexcept
    {
        return control()->shared_count;
    }

    void reset() noexcept
    {
        control_type* c = control();
        if(!c) return;

        if(--c->shared_count == 0)
        {
            // destroy object
        }

        // if not inline, we can simply blast control_ptr
        // to decouple and not manage anymore
        if(!control_inline.is_inline)
        {
            control_ptr = NULLPTR;
        }
        else
        {
            control_inline.is_active = false;
        }
    }
};

}

