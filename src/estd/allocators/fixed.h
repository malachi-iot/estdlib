#pragma once

#include "generic.h"
#include "../traits/allocator_traits.h"
#include "../internal/handle_with_offset.h"
#include <string.h> // for strlen
#include "../allocators/handle_desc.h"

namespace estd {

namespace internal {

// Can only have its allocate function called ONCE
// maps to one and only one regular non-locking buffer
// also this is a stateful allocator, by nature of TBuffer taking up some space
// if one wishes to be stateless, a different (base) class should be used
template <class T, class TBuffer, typename TSize>
struct single_allocator_base
{
    typedef const void* const_void_pointer;
    // really I want it an empty struct.  Bool is somewhat convenient though since we can
    // represent an invalid handle too
    typedef bool handle_type;
    typedef TSize size_type;
    typedef handle_type handle_with_size;
    //typedef T& handle_with_offset; // represents a pointer location past initial location of buffer
    typedef estd::internal::handle_with_only_offset<handle_type, size_type> handle_with_offset;
    typedef T value_type;
    typedef T* pointer;
    typedef estd::experimental::stateful_nonlocking_accessor<single_allocator_base> accessor;

protected:

    TBuffer buffer;

    // for inline buffers, we want the option of leaving it untouched
    single_allocator_base() {}

    single_allocator_base(const TBuffer& buffer) : buffer(buffer) {}

public:
    static CONSTEXPR handle_type invalid() { return false; }

    // technically we ARE locking since we have to convert the dummy 'bool' handle
    // to a pointer
    static CONSTEXPR bool is_locking() { return true; }

    static CONSTEXPR bool is_stateful() { return true; }

    static CONSTEXPR bool is_singular() { return true; }

    static CONSTEXPR bool has_size() { return true; }


    value_type& lock(handle_type h, int pos = 0, int count = 0)
    {
        return buffer[pos];
    }

    value_type& lock(const handle_with_offset& h, int pos = 0, int count = 0)
    {
        return buffer[h.offset() + pos];
    }

    const value_type& clock(handle_type h, int pos = 0, int count = 0) const
    {
        return buffer[pos];
    }

    const value_type& clock(const handle_with_offset& h, int pos = 0, int count = 0) const
    {
        return buffer[h.offset() + pos];
    }

    void unlock(handle_type h) {}

    void cunlock(handle_type h) const {}

    handle_with_offset offset(handle_type h, size_t pos) const
    {
        return handle_with_offset(h, pos);
    }

    void deallocate(handle_with_size h, size_type count)
    {
    }

    handle_type reallocate(handle_type h, size_t len)
    {
        // Not supported operation
        assert(false);

        return h;
    }


    typedef typename nothing_allocator<T>::lock_counter lock_counter;
};

// Can only have its allocate function called ONCE
// tracks how much of the allocator has been allocated
// null_terminated flag mainly serves as a trait/clue to specializations
// len can == 0 in which case we're in unbounded mode
template <
        class T, size_t len, class TBuffer = T[len],
        class TSize = typename internal::deduce_fixed_size_t<len>::size_type>
struct single_fixedbuf_allocator : public
        single_allocator_base<T, TBuffer, TSize>
{
    typedef single_allocator_base<T, TBuffer, TSize> base_t;

    typedef T value_type;
    typedef bool handle_type; // really I want it an empty struct
    typedef handle_type handle_with_size;
    typedef typename base_t::size_type size_type;

public:
    single_fixedbuf_allocator() {}

    // FIX: something bizzare is happening here and base_t is ending
    // up as map_base during debug session
    single_fixedbuf_allocator(const TBuffer& buffer) : base_t(buffer) {}


    handle_with_size allocate_ext(size_t size)
    {
        // we can't be sure if alloc fails or succeeds with unspecified length (when TBuffer = *)
        // but since we are embedded-oriented, permit it to succeed and trust the programmer
        // took precautions
        if(len == 0)   return true;

        // remember our handle is a true/false
        return size <= len;
    }

    handle_type allocate(size_t size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_t size)
    {
        // NOTE: assuming incoming handle_type is valid

        return allocate_ext(size);
    }

    // FIX: returns actual maximum size of unsigned,not ideal
    size_t max_size() const { return len == 0 ? -1 : len; }

    size_t size(handle_with_size h) const { return max_size(); }
};


// mainly for layer3:
// runtime (but otherwise constant) size()
// runtime (but otherwise constant) buffer*
// as before, null_terminated is merely a clue/trait for consumer class
template <class T, class TSize = std::size_t>
class single_fixedbuf_runtimesize_allocator : public single_allocator_base<T, T*, TSize>
{
public:
    typedef single_allocator_base<T, T*, TSize> base_t;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::handle_type handle_type;
    typedef handle_type handle_with_size;

private:
    size_type m_buffer_size;

public:
    struct InitParam
    {
        T* buffer;
        size_type size;

        InitParam(T* buffer, size_type size) : buffer(buffer), size(size) {}
    };

    single_fixedbuf_runtimesize_allocator(const InitParam& p)
        : base_t(p.buffer), m_buffer_size(p.size)
    {}

    size_type size(handle_with_size h) const { return m_buffer_size; }

    // FIX: This does *NOT* belong here, only a helper should do this
    // null_teriminated flag in the allocator is merely a trait
    size_type max_size() const
    {
        return m_buffer_size;
        //return m_buffer_size - (null_terminated ? 1 : 0);
    }

    handle_with_size allocate_ext(size_t size)
    {
        // TODO: put in a flag only in debug mode to detect multiple alloc
        // (should only have one for single+fixed allocator)

        // remember our handle is a true/false
        return size <= max_size();
    }

    handle_type allocate(size_t size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_t size)
    {
        // NOTE: assuming incoming handle_type is valid

        return allocate_ext(size);
    }
};

}

#ifndef FEATURE_CPP_CONSTEXPR
template <class T, size_t N, class TBuffer, class TSize, class TTraits>
struct handle_descriptor<internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>, TTraits >
        : internal::handle_descriptor_base<
                internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>,
                true,
                true,
                true>

{
    typedef internal::handle_descriptor_base<
            internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>,
            true,
            true,
            true> base_t;

    handle_descriptor() : base_t(true) {}

    template <class TAllocatorParam>
    handle_descriptor(const TAllocatorParam& p) : base_t(p, true) {}
};


template <class T, class TSize, class TTraits>
struct handle_descriptor<internal::single_fixedbuf_runtimesize_allocator<T, TSize>, TTraits >
        : internal::handle_descriptor_base<
                internal::single_fixedbuf_runtimesize_allocator<T, TSize>,
                true,
                true,
                true>

{
    typedef internal::handle_descriptor_base<
            internal::single_fixedbuf_runtimesize_allocator<T, TSize>,
            true,
            true,
            true> base_t;

    handle_descriptor() : base_t(true) {}

    template <class TAllocatorParam>
    handle_descriptor(const TAllocatorParam& p) :
//            base_t(p, typename TTraits::invalid_handle()) {}
            base_t(p, true) {}
};
#endif


}
