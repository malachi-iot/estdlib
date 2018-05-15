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
    friend std::basic_ostream<CharT, Traits>&  ::operator<<(std::basic_ostream<CharT, Traits>& os,
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

    size_type copy(value_type* dest, size_type count, size_type pos = 0) const
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


    template <class TForeignAllocator>
    basic_string& append(const basic_string<CharT, Traits, TForeignAllocator>& str)
    {
        size_type len = str.length();

        const CharT* append_from = str.fake_const_lock();

        base_t::_append(append_from, len);

        str.fake_const_unlock();

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


    template <class TForeignAlloc>
    int compare(const basic_string<CharT, Traits, TForeignAlloc>& str) const
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


    template <class ForeignAllocator>
    basic_string& operator=(const basic_string<CharT, Traits, ForeignAllocator>& copy_from)
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

#ifdef UNUSED
// TODO: consolidate with estd::string itself now that allocator is beefing up
// still have to crack the nut of the '0-byte' vs '1-byte' empty structs, which
// size_tracker_nullterm and size_tracker_default are attempting (but not yet
// succeeding) to do
template <class CharT, class TBuffer,
          class Traits = std::char_traits<CharT>,
          class StringTraits = null_terminated_string_traits>
class basic_string_base
{
protected:
    TBuffer buffer;
    typedef StringTraits straits_t;

public:
    // temporarily allowing const char* to char* since basic_string_base is going away anyway
    template <class TBufferParam>
    basic_string_base(TBufferParam in) : buffer((char*)in) {}

    typedef CharT value_type;
    typedef size_t size_type;

    // n is the 'at most' specifier. -1 means no upper bound
    // assumes null termination
    basic_string_base& append(const value_type* s, int n = -1)
    {
        if(straits_t::is_null_terminated())
        {
            if(n == -1)
                strcat(buffer, s);
            else
                strncat(buffer, s, n);
        }

        return *this;
    }

    template <class TString>
    basic_string_base& operator += (TString s)
    {
        return append(s);
    }


    bool operator==(const basic_string_base& compare_to) const
    {
        return strcmp(buffer, compare_to.buffer) == 0;
    }


    bool operator!=(const basic_string_base& compare_to) const
    {
        return strcmp(buffer, compare_to.buffer) != 0;
    }


    bool operator>(const basic_string_base& compare_to) const
    {
        return strcmp(buffer, compare_to.buffer) > 0;
    }

    // FIX
    const TBuffer& raw() const { return buffer; }
};




namespace layer2 {

// represents a null-terminated string with a constexpr-size.  Should be nearly identical to
// a layer3 variety with a non-constexpr-size
// experimental because even though this is quite possible, combining it with regular basic_string
// is both desired but complicated
// TODO: Probably we want to distinguish this as null-terminated vs not null terminated either with
// traits or an explicit name
// N defaults to 0, kind of ugly by C++ standards but the norm for C - strings have an unspecified
// upper bound
template <class CharT, size_t N = 0, class Traits = std::char_traits<CharT > >
class basic_string : public basic_string_base<CharT, CharT*, Traits>
{
protected:
    typedef basic_string_base<CharT, CharT*, Traits> base_t;

public:
    typedef CharT value_type;
    typedef size_t size_type;

    basic_string(const CharT* buffer) : base_t(buffer) {}

    //basic_string(const basic_string& copy_from) : base_t(copy_from.buffer) {}

#ifdef FEATURE_CPP_STATIC_ASSERT
    basic_string(const base_t& copy_from) : base_t(copy_from.buffer)
    {
        static_assert(N == 0, "May only initialize when length is unspecified");
    }
#endif


    size_type size() const
    {
        return strlen(base_t::buffer);
    }

    size_type length() const { return size(); }

    size_type max_size() const { return N - 1; }

    size_type copy(value_type* dest, size_type count, size_type pos = 0)
    {
        // TODO: Do bounds checking etc.
        strncpy(dest, base_t::buffer + pos, count);
        return count;
    }

    // "lossy" conversion to make basic_string<CharT> more
    // passable analgous to classic C char* strings
    operator basic_string<CharT, 0, Traits>() const
    {
        return basic_string<CharT, 0, Traits>(base_t::buffer);
    }
};

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N = 0>
using string = basic_string<char, N>;
#endif


}


namespace layer3 {

// represents a null-terminated string with a particular specified size
// TODO: Probably we want to distinguish this as null-terminated vs not null terminated either with
// traits or an explicit name
template <class CharT, class Traits = std::char_traits<CharT> >
class basic_string : public basic_string_base<CharT, CharT*, Traits>
{
public:
    typedef size_t size_type;

protected:
    size_type m_length;
    typedef basic_string_base<CharT, CharT*> base_t;

public:
    typedef CharT value_type;

    template <size_t N>
    basic_string(const layer2::basic_string<CharT, N>& copy_from)
        : base_t(copy_from.raw()), m_length(copy_from.max_size())
    {

    }

    template <size_t N>
    basic_string(CharT (&buffer) [N]) :
        base_t(buffer), m_length(N) {}

    basic_string(CharT* buffer, size_type length) :
        base_t(buffer), m_length(length)
    {}

    size_type size() const
    {
        return strlen(base_t::buffer);
    }

    size_type length() const { return size(); }

    size_type max_size() const { return m_length - 1; }

    size_type copy(value_type* dest, size_type count, size_type pos = 0)
    {
        // TODO: Do bounds checking etc.
        strncpy(dest, base_t::buffer + pos, count);
        return count;
    }
};
}
#endif
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

    template <class ForeignAllocator>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator>& copy_from)
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

    /*
     * Something seems to go wrong with allocator initialization on this one
    template <class ForeignAllocator>
    basic_string(const estd::basic_string<CharT, Traits, ForeignAllocator> & copy_from) : base_t(copy_from)
    {

    } */

    template <class ForeignAllocator>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N = 0>
using string = basic_string<char, N>;
#endif

typedef basic_string<char, 0, true, std::char_traits<char>, const char*> const_string;


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
    typedef typename base_t::size_type size_type;

public:
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
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


// defaults to null-terminated variety
typedef basic_string<char> string;


}


template <class CharT, class Traits, class Alloc>
bool operator ==( const CharT* lhs, const basic_string<CharT, Traits, Alloc>& rhs)
{
    return rhs.compare(lhs) == 0;
};

template <class CharT, class Traits, class Alloc>
bool operator ==( const basic_string<CharT, Traits, Alloc>& lhs, const CharT* rhs)
{
    return lhs.compare(rhs) == 0;
};

template <class CharT, class Traits, class AllocLeft, class AllocRight>
bool operator ==( const basic_string<CharT, Traits, AllocLeft>& lhs,
                  const basic_string<CharT, Traits, AllocRight>& rhs)
{
    return lhs.compare(rhs) == 0;
}


}

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
#endif

