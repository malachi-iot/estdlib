#pragma once

#include <cstddef> // for size_t

namespace estd { namespace internal {

template <typename size_t>
class handle_with_only_offset
{
    size_t m_offset;

public:
    handle_with_only_offset(size_t offset) : m_offset(offset) {}

    size_t offset() const { return m_offset; }
};


template <class THandle, typename size_type = std::size_t>
class handle_with_offset
{
    THandle m_handle;
    size_type m_offset;

public:
    handle_with_offset(THandle h, size_type offset) :
        m_handle(h),
        m_offset(offset) {}

    std::ptrdiff_t offset() const { return m_offset; }
    THandle handle() const { return m_handle; }

    bool operator ==(const handle_with_offset& compare_to) const
    {
        return m_handle == compare_to.m_handle && offset() == compare_to.offset();
    }

    // TODO: Make allocator version which does bounds checking
    void increment(size_type count = 1)
    {
        m_offset += count;
    }
};


template <class TPointer, typename size_t = std::size_t>
class handle_with_offset_raw
{
    TPointer m_pointer;

public:
    handle_with_offset_raw(TPointer p) : m_pointer(p) {}

    std::ptrdiff_t offset() const { return m_pointer; }
    TPointer handle() const { return m_pointer; }

    bool operator ==(const handle_with_offset_raw& compare_to) const
    {
        return m_pointer == compare_to.m_pointer;
    }

    // TODO: Make allocator version which does bounds checking
    void increment(size_t count = 1)
    {
        m_pointer += count;
    }
};

}}
