#pragma once

#include "platform.h"
#include "../cstddef.h" // for size_t
#include "impl/handle_desc.h"

namespace estd { namespace internal {

// beware this will permit different handles themselves to do math operations
// on each other.  Obviously undefined, but it seems no more dangerous than
// stock standard iterator math
template <typename size_t>
class handle_with_offset_base
{
protected:
    size_t m_offset;

    handle_with_offset_base(size_t m_offset) :
            m_offset(m_offset) {}

public:
    typedef size_t size_type;

    // TODO: Make allocator version which does bounds checking
    // TODO: phase out increment if we can in favor of += operator
    void increment(size_type count = 1)
    {
        m_offset += count;
    }

    inline handle_with_offset_base& operator+=(size_type summand)
    {
        m_offset += summand;
        return *this;
    }

    inline handle_with_offset_base& operator-=(size_type subtrahend)
    {
        m_offset -= subtrahend;
        return *this;
    }

    ptrdiff_t operator-(const handle_with_offset_base& subtrahend) const
    {
        return m_offset - subtrahend.m_offset;
    }

    // NOTE: be mindful that these comparisons presume either:
    // a) a matching handle, if one exists
    // b) a non-matching handle whose non-matching nature doesn't matter, if one exists
    // c) no handle at all (dummy handle, like fixed allocator)
    // if none of these are the case, be sure to overload these operators
    // in inherited classes
    bool operator >(const handle_with_offset_base& compare_to) const
    {
        return m_offset > compare_to.m_offset;
    }

    bool operator==(const handle_with_offset_base& compare_to) const
    {
        return m_offset == compare_to.m_offset;
    }
};

// typically this will only be bool for THandle, but feel free
// to specialize for other conditions if you need to
template<class THandle, typename size_t = std::size_t>
class handle_with_only_offset;

// 'fake' handle (typically used with singular-allocator) with explicit offset
template<typename size_t>
class handle_with_only_offset<bool, size_t> :
        public handle_with_offset_base<size_t>
{
    typedef handle_with_offset_base<size_t> base_t;

public:
    handle_with_only_offset(size_t offset) :
            base_t(offset) {}

    // TODO: Assert that incoming h == true
    handle_with_only_offset(bool h, size_t offset) :
            base_t(offset) {}

    size_t offset() const { return base_t::m_offset; }

    bool handle() const
    {
        return true;
    }
};

#ifdef FEATURE_ESTD_ENHANCED_HANDLE_EXP
struct single_allocator_handle
{
    bool is_valid;
    CONSTEXPR single_allocator_handle(bool is_valid) : is_valid(is_valid) {}

    operator bool() const { return is_valid; }
};

// 'fake' handle (typically used with singular-allocator) with explicit offset
template<typename size_t>
class handle_with_only_offset<single_allocator_handle, size_t> :
        public handle_with_offset_base<size_t>
{
    typedef handle_with_offset_base<size_t> base_t;

public:
    handle_with_only_offset(size_t offset) :
            base_t(offset) {}

    // TODO: Assert that incoming h == true
    handle_with_only_offset(single_allocator_handle h, size_t offset) :
            base_t(offset) {}

    size_t offset() const { return base_t::m_offset; }

    single_allocator_handle handle() const
    {
        return true;
    }
};
#endif

// 'traditional' handle with offset with a proper handle and explicit offset
template<class THandle, typename size_type = std::size_t>
class handle_with_offset :
        public handle_with_offset_base<size_type>
{
    typedef handle_with_offset_base<size_type> base_t;

    THandle m_handle;

public:
    handle_with_offset(THandle h, size_type offset) :
            m_handle(h),
            base_t(offset) {}

    std::ptrdiff_t offset() const { return base_t::m_offset; }

    THandle handle() const { return m_handle; }

    bool operator==(const handle_with_offset& compare_to) const
    {
        return m_handle == compare_to.m_handle && offset() == compare_to.offset();
    }
};


// handle_with_offset class which actually contains a pointer
template<class TPointer, typename size_t = std::size_t>
class handle_with_offset_raw
{
    TPointer m_pointer;

public:
    handle_with_offset_raw(TPointer p) : m_pointer(p) {}

    std::ptrdiff_t offset() const { return m_pointer; }

    TPointer handle() const { return m_pointer; }

    bool operator==(const handle_with_offset_raw& compare_to) const
    {
        return m_pointer == compare_to.m_pointer;
    }

    // TODO: Make allocator version which does bounds checking
    void increment(size_t count = 1)
    {
        m_pointer += count;
    }

    inline handle_with_offset_raw& operator +=(size_t summand)
    {
        m_pointer += summand;
        return *this;
    }

    inline std::ptrdiff_t operator -(const handle_with_offset_raw& subtrahend) const
    {
        return m_pointer - subtrahend.m_pointer;
    }
};



}}
