#pragma once

#include "../fwd/dynamic_array.h"
#include "../fwd/string.h"
#include "../../traits/allocator_traits.h"
#include "../../algorithm.h"

#include "ends_with.h"
#include "starts_with.h"

#include <string.h>     // for access to memcpy NOLINT

namespace estd { namespace internal {

/// Handles special cases of how allocated_array impls
/// like to copy/assign/compare bulk data
template <class Impl, class Enabled = void>
struct dynamic_array_helper;


template <class Impl>
struct dynamic_array_helper<Impl, enable_if_t<
    Impl::allocator_traits::locking_preference == allocator_locking_preference::iterator> >
{
    typedef internal::dynamic_array<Impl> dynamic_array;
    typedef internal::allocated_array<Impl> array;

    typedef typename array::value_type value_type;
    typedef typename array::pointer pointer;
    typedef typename array::const_pointer const_pointer;
    typedef typename array::size_type size_type;
    typedef typename array::iterator iterator;
    typedef typename array::const_iterator const_iterator;

    // copy from us to outside dest/other
    static size_type copy_to(const array& a,
        typename estd::remove_const<value_type>::type* dest,
        size_type count, size_type pos = 0)
    {
        auto src = a.create_iterator(pos);

        // TODO: get_allocator must be present in pgm flavor

        // TODO: since we aren't gonna throw an exception, determine what to do if
        // pos > size()

        if(pos + count > a.size())
            count = a.size() - pos;

        estd::copy_n(src, count, dest);

        return count;
    }


    /// Low-level compare - does NOT check for matching size!
    template <class Impl2>
    static bool equal(const array& lhs, const allocated_array<Impl2>& rhs,
        size_type len)
    {
        const_iterator it = lhs.begin();
        return estd::equal(it, it + len, rhs.begin());
    }
};

template <class Impl>
struct dynamic_array_helper<Impl, enable_if_t<
    Impl::allocator_traits::locking_preference == allocator_locking_preference::standard ||
    Impl::allocator_traits::locking_preference == allocator_locking_preference::none> >
{
    typedef internal::dynamic_array<Impl> dynamic_array;
    typedef internal::allocated_array<Impl> array;

    array& a;

    typedef typename array::value_type value_type;
    typedef typename array::pointer pointer;
    typedef typename array::const_pointer const_pointer;
    typedef typename array::size_type size_type;

    //pointer raw_;

    ESTD_CPP_CONSTEXPR_RET EXPLICIT dynamic_array_helper(array& a) :
        a(a)
    {
    }

    // from = other -> this
    // to = this -> other

    // copies into da at specified pos
    static void copy_from(array& da, unsigned pos, const_pointer from, unsigned len)
    {
        pointer raw = da.lock(pos);

        estd::copy_n(from, len, raw);

        da.unlock();
    }

    // copies into da a specified allocated array
    template <class Impl2>
    static void copy_from(array& da, unsigned pos, const allocated_array<Impl2>& from, unsigned len)
    {
        pointer raw = da.lock(pos);

        from.copy(raw, len);

        da.unlock();
    }

    // copies into da from first to last
    template <class InputIt>
    static void copy_from(array& da, unsigned pos, InputIt first, InputIt last)
    {
        pointer raw = da.lock(pos);

        estd::copy(first, last, raw);

        da.unlock();
    }

    // DEBT: Temporary, move the bulk of this back out to dynamic_array and turn
    // this into a regular 'array' flavor
    template <class Impl2>
    //static typename dynamic_array::append_result
    // DEBT: This return type won't always match, depending on feature flags
    static size_type
        append(dynamic_array& a, const allocated_array<Impl2>& copy_from)
    {
        size_type len = copy_from.size();
        auto r = a.grow(len);
        const size_type current_size = r.starting_size;
        pointer raw = a.lock(current_size);

        copy_from.copy(raw, len);

        a.unlock();

        return {};
    }

    template <class Impl2>
    static bool starts_with(const array& a, const allocated_array<Impl2>& compare_to)
    {
        const value_type* s = a.clock();

        // DEBT: Try not to presume rhs/compare_to is lockable
        const value_type* t = compare_to.clock();

        bool r = starts_with_n(s, t, a.size(), compare_to.size());

        a.cunlock();
        compare_to.cunlock();

        return r;
    }


    template <class Impl2>
    static bool ends_with(const array& a, const allocated_array<Impl2>& compare_to)
    {
        const_pointer s = a.clock();

        // DEBT: Try not to presume rhs/compare_to is lockable
        const_pointer t = compare_to.clock();

        bool r = ends_with_n(s, t, a.size(), compare_to.size());

        a.cunlock();
        compare_to.cunlock();

        return r;
    }


    // copy from us to outside dest/other
    static size_type copy_to(const array& a,
        typename estd::remove_const<value_type>::type* dest,
        size_type count, size_type pos = 0)
    {
        const value_type* src = a.clock(pos, count);

        // TODO: since we aren't gonna throw an exception, determine what to do if
        // pos > size()

        if(pos + count > a.size())
            count = a.size() - pos;

        // TODO: Do the m_impl.copy_from here

        memcpy(dest, src, count * sizeof(value_type));

        a.cunlock();

        return count;
    }

    /// Low-level compare - does NOT check for matching size!
    template <class Impl2>
    ESTD_CPP_CONSTEXPR(14) static bool equal(const array& lhs, const allocated_array<Impl2>& rhs,
        size_type len)
    {
        // gets here if size matches
        const value_type* raw = lhs.clock();

        // DEBT: we don't want to presume rhs can lock
        const value_type* s = rhs.clock();

        bool result = estd::equal(raw, raw + len, s);

        lhs.cunlock();
        rhs.cunlock();

        return result;
    }


    /// Low-level lexicographic compare - does NOT check for matching size!
    /// Helper for https://en.cppreference.com/cpp/string/char_traits/compare
    ESTD_CPP_CONSTEXPR(14) static int compare(const detail::basic_string<Impl>& lhs, const_pointer s, size_type s_size)
    {
        using traits_type = typename detail::basic_string<Impl>::traits_type;

        const_pointer raw = lhs.clock();

        const int result = traits_type::compare(raw, s, s_size);

        lhs.cunlock();

        return result;
    }


    /// Low-level lexicographic compare - DOES check for matching size and presumes lhs is null terminated
    /// Helper for https://en.cppreference.com/cpp/string/basic_string/compare
    template <class RhsIt>
    ESTD_CPP_CONSTEXPR(14) static int compare_null_term(const detail::basic_string<Impl>& lhs, RhsIt rhs, size_type s_size)
    {
        static_assert(detail::basic_string<Impl>::is_null_terminated, "Requires null terminated");

        const_pointer raw = lhs.clock();
        int our_pos = 0;

        for(;*raw != 0; ++rhs, ++raw, ++our_pos)
        {
            // if our_pos (0 based) == s_size (somewhat 1 based) that means we've gone
            // one past rhs, meaning we qualify as larger
            if(our_pos == s_size)   return 1;

            int r = *raw - *rhs;

            if(r != 0) return r;
        }

        lhs.cunlock();

        // Reaching here means our_pos (grown to 1 based) <= s_size after an otherwise lexicographic match
        return our_pos - s_size;
    }

    // Similar to above, but both lhs and rhs are null terminated
    // UNTESTED, UNUSED
    template <class RhsIt>
    ESTD_CPP_CONSTEXPR(14) static int compare_null_term(const detail::basic_string<Impl>& lhs, RhsIt rhs)
    {
        static_assert(detail::basic_string<Impl>::is_null_terminated, "Requires null terminated");

        const_pointer raw = lhs.clock();

        for(;*raw != 0; ++rhs, ++raw)
        {
            // If we reach end rhs first, that means we're a bigger string, meaning
            // we are "larger"
            if(*rhs == 0) return 1;

            int r = *raw - *rhs;

            if(r != 0) return r;
        }

        lhs.cunlock();

        // If we both are null terminated, return a 0
        // Otherwise we are "smaller" than rhs, return some value < 0
        return *raw - *rhs;
    }

    // rhs = null terminated C string
    template <class InputIt>
    ESTD_CPP_CONSTEXPR(17) static bool starts_with(const array& lhs, InputIt rhs)
    {
        const_pointer s = lhs.clock();

        bool r = starts_with_n(s, rhs, lhs.size());

        lhs.cunlock();

        return r;
    }


    // rhs = null terminated C string
    template <class InputIt>
    ESTD_CPP_CONSTEXPR(17) static bool ends_with(const array& lhs, InputIt rhs)
    {
        const_pointer s = lhs.clock();

        bool r = ends_with_n(s, rhs, lhs.size(), strlen(rhs));

        lhs.cunlock();

        return r;
    }
};


}}
