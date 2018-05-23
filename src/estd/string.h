#pragma once

#include "memory.h"
#include "internal/dynamic_array.h"
#include "allocators/fixed.h"
#include <algorithm> // for std::min

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
#include <ostream>
#endif

namespace estd {

/*
template<class CharT> struct char_traits;

template<> struct char_traits<char>
{
    typedef char char_type;
    typedef int int_type;
    typedef char nonconst_char_type;
};


template<> struct char_traits<const char>
{
    typedef const char char_type;
    typedef int int_type;
    typedef char nonconst_char_type;
}; */

// prototype
template<
    class CharT,
    class Traits,
    class Allocator
> class basic_string;

}

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
template <class CharT, class Traits, class Allocator>
std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
               const estd::basic_string<CharT, Traits, Allocator>& str);
#endif

namespace estd {

// TODO: Determine how to organize different string implementations
// a) wrapper around standard C null terminated variety
// b) wrapper around pascal-style length tracking variety (which we'll also combine with dynamic allocation)
// thinking a) would be a good layer2 string and b) would be a good layer3
// We can start switching 'layer' version of string to derive from basic_string itself by using
// fixed allocators
template<
    class CharT,
    class Traits = std::char_traits<CharT>,
    class Allocator = std::allocator<CharT>
> class basic_string :
        public internal::dynamic_array<Allocator>
{
    typedef internal::dynamic_array<Allocator> base_t;
    typedef basic_string<CharT, Traits, Allocator> this_t;

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
    template <class TChar2, class TTraits2>
    friend std::basic_ostream<TChar2, TTraits2>&  ::operator<<(
            std::basic_ostream<TChar2, TTraits2>& os,
            const estd::basic_string<CharT, Traits, Allocator>& str);
#endif

public:
    typedef typename base_t::size_type size_type;

protected:
    template <class THelperParam>
    basic_string(const THelperParam& p) : base_t(p) {}

public:
    basic_string() {}

    template <class ForeignAllocator>
    basic_string(const basic_string<CharT, Traits, ForeignAllocator>& copy_from)
    {
        operator =(copy_from);
    }

    typedef CharT value_type;
    typedef Traits traits_type;
    typedef Allocator allocator_type;

    typedef typename allocator_type::handle_type handle_type;

    size_type length() const { return base_t::size(); }

    basic_string& erase(size_type index = 0, size_type count = -1)
    {
        size_type to_remove_count = std::min(count, base_t::size() - index);

        base_t::_erase(index, to_remove_count);

        return *this;
    }

    size_type copy(typename estd::remove_const<value_type>::type* dest,
                   size_type count, size_type pos = 0) const
    {
        const value_type* src = base_t::fake_const_lock();

        // TODO: since we aren't gonna throw an exception, determine what to do if
        // pos > size()

        if(pos + count > length())
            count = length() - pos;

        memcpy(dest, src + pos, count);

        base_t::fake_const_unlock();

        return count;
    }

    basic_string& append(size_type count, value_type c)
    {
        while(count--) base_t::push_back(c);

        return *this;
    }


    template <class TForeignChar, class TForeignTraits, class TForeignAllocator>
    basic_string& append(const basic_string<TForeignChar, TForeignTraits, TForeignAllocator>& str)
    {
        base_t::append(str);
        return *this;
    }

    basic_string& append(const value_type* s)
    {
        size_t len = strlen(s);

        base_t::_append(s, len);

        return *this;
    }


    int compare( const CharT* s ) const
    {
        size_type raw_size = base_t::size();
        size_type s_size = strlen(s);

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const CharT* raw = base_t::fake_const_lock();

        int result = traits_type::compare(raw, s, raw_size);

        base_t::fake_const_unlock();

        return result;

    }


    template <class TForeignChar, class TForeignTraits, class TForeignAlloc>
    int compare(const basic_string<TForeignChar, TForeignTraits, TForeignAlloc>& str) const
    {
        size_type raw_size = base_t::size();
        size_type s_size = str.size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const CharT* raw = base_t::fake_const_lock();
        const CharT* s = str.fake_const_lock();

        int result = traits_type::compare(raw, s, raw_size);

        base_t::fake_const_unlock();

        str.fake_const_unlock();

        return result;
    }


    template <class TString>
    basic_string& operator += (TString s)
    {
        return append(s);
    }


    basic_string& operator += (value_type c)
    {
        base_t::push_back(c);
        return *this;
    }


    // using foreignchar and foreigntraits primarily to interact with const char
    template <class ForeignCharT, class ForeignTraitsT, class ForeignAllocator>
    basic_string& operator=(const basic_string<ForeignCharT, ForeignTraitsT, ForeignAllocator>& copy_from)
    {
        base_t::reserve(copy_from.size());
        base_t::helper.size(copy_from.size());
        // remember, capacity indicates the biggest size
        // the this->buffer can be
        size_type capacity = base_t::capacity();
        // copy from copy_from into our own buffer
        copy_from.copy(base_t::lock(), capacity);
        base_t::unlock();
        return *this;
    }

    basic_string& operator=(const CharT* s)
    {
        base_t::assign(s, strlen(s));
        return *this;
    }
};


typedef basic_string<char> string;

namespace experimental {


template <class CharT, class TCharTraits = std::char_traits<CharT > >
class string_traits
{
    static bool is_null_terminated();

    static bool is_null_terimnation(const CharT& value);
};


struct null_terminated_string_traits
{
    static bool is_null_terminated() { return true; }

    static bool is_null_termination(const char& value) { return value == 0; }

    static size_t length(char* str) { return strlen(str); }
};


}


namespace layer1 {


template<class CharT, size_t N, bool null_terminated = true, class Traits = std::char_traits<CharT > >
class basic_string
        : public estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated> >
{
    typedef estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated> >
                base_t;
public:
    basic_string() {}

    basic_string(const CharT* s)
    {
        base_t::operator =(s);
    }

    // using ForeignCharT and ForeignTraits because incoming string might use const char
    template <class ForeignCharT, class ForeignTraits, class ForeignAllocator>
    basic_string& operator=(const estd::basic_string<ForeignCharT, ForeignTraits, ForeignAllocator>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N>
using string = basic_string<char, N>;
#endif

}


// TODO: refactor layer2 & layer3 to utilize a handle and not a CharT*
namespace layer2 {

template<class CharT, size_t N, bool null_terminated = true, class Traits = std::char_traits<CharT>, class PCharT = CharT* >
class basic_string
        : public estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated, PCharT > >
{
    typedef estd::basic_string<
            CharT, Traits,
            estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated, PCharT > >
            base_t;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::helper_type helper_type;
    typedef typename base_t::size_type size_type;

public:
    basic_string(const CharT* str_buffer) : base_t(str_buffer)
    {
        // TODO: optimize this explicit strlen out
        //base_t::assign(str_buffer, strlen(str_buffer));
    }

    /*
    basic_string(CharT* str_buffer) : base_t(str_buffer)
    {

    } */

    template <size_type IncomingN>
    basic_string(CharT (&buffer) [IncomingN]) : base_t(&buffer[0])
    {
        static_assert(IncomingN >= N || N == 0, "Incoming buffer size incompatible");
    }

    template <class ForeignAllocator>
    basic_string(const estd::basic_string<CharT, Traits, ForeignAllocator> & copy_from)
        // FIX: very bad -- don't leave things locked!
        // only doing this because we often pass around layer1, layer2, layer3 strings who
        // don't care about lock/unlock
        : base_t(copy_from.fake_const_lock())
    {
    }

    template <class ForeignAllocator>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N = 0, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;
#endif

typedef basic_string<const char, 0> const_string;


}

namespace layer3 {

template<class CharT, bool null_terminated = true, class Traits = std::char_traits<CharT> >
class basic_string
        : public estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_runtimesize_allocator < CharT, null_terminated > >
{
    typedef estd::basic_string<
            CharT, Traits,
            estd::internal::single_fixedbuf_runtimesize_allocator < CharT, null_terminated > >
            base_t;

    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::helper_type helper_type;

protected:
    typedef typename base_t::size_type size_type;
    typedef typename allocator_type::InitParam init_t;

public:
    template <size_type N>
    basic_string(const CharT (&buffer) [N], bool source_null_terminated = true) :
        base_t(typename allocator_type::InitParam(buffer, N))
    {
        base_t::helper.size(N - (source_null_terminated ? 1 : 0));
    }

    template <size_type N>
    basic_string(size_type initial_size, CharT (&buffer) [N]) :
        base_t(typename allocator_type::InitParam(buffer, N))
    {
        base_t::helper.size(initial_size);
    }

    basic_string(size_type initial_size, CharT* buffer, size_type buffer_size) :
        base_t(typename allocator_type::InitParam(buffer, buffer_size))
    {
        base_t::helper.size(initial_size);
    }

    template <class ForeignAllocator>
    basic_string(const estd::basic_string<CharT, Traits, ForeignAllocator> & copy_from)
        // FIX: very bad -- don't leave things locked!
        // only doing this because we often pass around layer1, layer2, layer3 strings who
        // don't care about lock/unlock
        : base_t(init_t(copy_from.fake_const_lock(), copy_from.max_size()))
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert(helper_type::uses_termination(), "Only supports null terminated at this time");
#endif

        // can't do following right now since compiler still tries to compile null
        // terminated flavor for this, which will fail when using const char
        /*
        // If null terminated, don't set size as
        // a) it's already set in the incoming buffer
        // b) we'd be modifying original buffer - though in theory just rewriting
        //    0 back onto the existing 0
        if(!helper_type::uses_termination())
        {
            base_t::helper.size(copy_from.size());
        } */
    }


    template <class ForeignAllocator>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


// defaults to null-terminated variety
typedef basic_string<char> string;

// Non-NULL-terminated const strings use layer3
// NULL-terminated const strings use layer2
class const_string : public basic_string<const char, false>
{
    typedef basic_string<const char, false> base_t;
    typedef typename base_t::size_type size_type;

public:
    const_string(const char* s, size_type len) :
        base_t(len, s, len) {}

    template <size_type N>
    const_string(const char (&buffer) [N], bool source_null_terminated = true) :
        base_t(buffer, source_null_terminated) {}
};


}


template <class CharT, class Traits, class Alloc>
bool operator ==( const CharT* lhs, const basic_string<CharT, Traits, Alloc>& rhs)
{
    return rhs.compare(lhs) == 0;
}

template <class CharT, class Traits, class Alloc>
bool operator ==( const basic_string<CharT, Traits, Alloc>& lhs, const CharT* rhs)
{
    return lhs.compare(rhs) == 0;
}


// NOTE: Not sure why we need this particular one
template <class CharT, class Traits, class Alloc>
bool operator ==( const basic_string<const CharT, Traits, Alloc>& lhs, const CharT* rhs)
{
    return lhs.compare(rhs) == 0;
}


template <class TraitsLeft,
          class TraitsRight,
          class AllocLeft, class AllocRight>
bool operator ==( const basic_string<typename TraitsLeft::char_type, TraitsLeft, AllocLeft>& lhs,
                  const basic_string<typename TraitsRight::char_type, TraitsRight, AllocRight>& rhs)
{
    return lhs.compare(rhs) == 0;
}


}


#include "internal/to_string.h"

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
template <class CharT, class Traits, class Allocator>
inline std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
               const estd::basic_string<CharT, Traits, Allocator>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.fake_const_lock(), str.size());

    str.fake_const_unlock();

    return os;
}


// FIX: Temporarily using this, but really managing interaction between two sets
// of traits is a better way to go, ala:
// template <class CharT, class Traits, class Allocator,
//  class OStreamTraits = std::char_traits<remove_const<Traits::char_type>>>
template <class CharT, class Traits, class Allocator>
inline std::ostream&
    operator<<(std::ostream& os,
               const estd::basic_string<const CharT, Traits, Allocator>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.fake_const_lock(), str.size());

    str.fake_const_unlock();

    return os;
}
#endif

