#pragma once

#include "memory.h"
#include "allocators/fixed.h"
#include "internal/dynamic_array.h"
#include "traits/string.h"
#include <algorithm> // for std::min

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
#include <ostream>
#endif

namespace estd {

// prototype
template<
    class CharT,
    class Traits,
    class Allocator,
    class StringTraits
> class basic_string;

}

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
template <class CharT, class Traits, class Allocator, class StringTraits>
std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
               const estd::basic_string<CharT, Traits, Allocator, StringTraits>& str);
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
    class Traits = std::char_traits<typename estd::remove_const<CharT>::type >,
    class Allocator = std::allocator<CharT>,
    class StringTraits = experimental::string_traits<Traits>
> class basic_string :
        public internal::dynamic_array<Allocator, internal::impl::dynamic_array<Allocator> >
{
    typedef internal::dynamic_array<Allocator> base_t;
    typedef basic_string<CharT, Traits, Allocator, StringTraits> this_t;

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
    typedef StringTraits string_traits_type;

    typedef typename allocator_type::handle_type handle_type;

    size_type length() const { return base_t::size(); }

    basic_string& erase(size_type index = 0, size_type count = -1)
    {
        size_type to_remove_count = std::min(count, base_t::size() - index);

        base_t::_erase(index, to_remove_count);

        return *this;
    }

    basic_string& append(size_type count, value_type c)
    {
        while(count--) base_t::push_back(c);

        return *this;
    }


    template <class TForeignChar, class TForeignTraits, class TForeignAllocator>
    basic_string& append(const basic_string<TForeignChar, typename TForeignTraits::char_traits, TForeignAllocator, TForeignTraits>& str)
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
        const CharT* raw = base_t::clock();

        int result = traits_type::compare(raw, s, raw_size);

        base_t::cunlock();

        return result;

    }


    template <class TForeignChar, class TForeignTraits, class TForeignAlloc>
    int compare(const basic_string<TForeignChar, typename TForeignTraits::char_traits, TForeignAlloc, TForeignTraits>& str) const
    {
        size_type raw_size = base_t::size();
        size_type s_size = str.size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        const CharT* raw = base_t::clock();
        const CharT* s = str.clock();

        int result = traits_type::compare(raw, s, raw_size);

        base_t::cunlock();

        str.cunlock();

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


    template <class THelper>
    basic_string& operator=(const internal::dynamic_array<
                                typename THelper::allocator_type,
                                THelper>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }

    basic_string& operator=(const CharT* s)
    {
        base_t::assign(s, strlen(s));
        return *this;
    }

    // compare to a C-style string
    bool starts_with(const CharT* compare_to) const
    {
        const value_type* s = base_t::clock();

        size_type source_max = length();

        while(source_max-- && *compare_to != 0)
            if(*s++ != *compare_to++)
            {
                base_t::cunlock();
                return false;
            }

        base_t::cunlock();
        // if compare_to is longer than we are, then it's also a fail
        return source_max != -1;
    }


    template <class THelper>
    bool starts_with(const internal::dynamic_array<
                                typename THelper::allocator_type,
                                THelper>& compare_to)
    {
        return base_t::starts_with(compare_to);
    }
};


typedef basic_string<char> string;



namespace layer1 {


template<class CharT, size_t N, bool null_terminated = true, class Traits = std::char_traits<CharT >,
        class StringTraits = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_traits<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_traits<Traits, int16_t, estd::is_const<CharT>::value> >::type
                >
class basic_string
        : public estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_allocator <CharT, N, null_terminated>,
                StringTraits>
{
    typedef estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated>,
                StringTraits>
                base_t;
public:
    basic_string() {}

    basic_string(const CharT* s)
    {
        base_t::operator =(s);
    }

    template <class TForeignAllocator, class TForeignTraits>
    basic_string(const estd::basic_string<
            CharT,
            typename TForeignTraits::char_traits,
            TForeignAllocator,
            TForeignTraits>& copy_from)
    {
        base_t::operator=(copy_from);
    };

    // using ForeignCharT and ForeignTraits because incoming string might use const char
    template <class ForeignCharT, class ForeignTraits, class ForeignAllocator>
    basic_string& operator=(const estd::basic_string<
                            ForeignCharT,
                            typename ForeignTraits::char_traits,
                            ForeignAllocator,
                            ForeignTraits>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N>
using string = basic_string<char, N>;
#else
template <size_t N>
class string : public basic_string<char, N> { };
#endif

}


// TODO: refactor layer2 & layer3 to utilize a handle and not a CharT*
namespace layer2 {

template<class CharT, size_t N, bool null_terminated = true,
         class Traits = std::char_traits<typename estd::remove_const<CharT>::type >,
         class StringTraits = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_traits<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_traits<Traits, int16_t, estd::is_const<CharT>::value> >::type >
class basic_string
        : public estd::basic_string<
                CharT,
                Traits,
                estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated, CharT* >,
                StringTraits >
{
    typedef estd::basic_string<
            CharT, Traits,
            estd::internal::single_fixedbuf_allocator < CharT, N, null_terminated, CharT* >,
            StringTraits >
            base_t;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::impl_type helper_type;
    typedef typename base_t::size_type size_type;

public:
    // n = -1 means treat str_buffer as pre-initialized and null-terminated
    // n >= 0 means copy n characters from str_buffer, ignoring any null termination
    //        0 is a useful value as it will auto set underlying string to 0
    // NOTE: Above not active yet
    // This particular constructor is good for string literals, but any const char* probably is fine
    basic_string(const CharT* str_buffer, int n = -1) : base_t(str_buffer)
    {
        // TODO: optimize this explicit strlen out
        //base_t::assign(str_buffer, strlen(str_buffer));
    }

    /*
    basic_string(CharT* str_buffer) : base_t(str_buffer)
    {

    } */

    // See 'n' documentation above
    template <size_type IncomingN>
    basic_string(CharT (&buffer) [IncomingN], int n = -1) : base_t(&buffer[0])
    {
        static_assert(IncomingN >= N || N == 0, "Incoming buffer size incompatible");

        if(n >= 0) base_t::impl().size(n);
    }

    template <class ForeignAllocator, class ForeignTraits>
    basic_string(const estd::basic_string<CharT, typename ForeignTraits::char_traits, ForeignAllocator, ForeignTraits> & copy_from)
        // FIX: very bad -- don't leave things locked!
        // only doing this because we often pass around layer1, layer2, layer3 strings who
        // don't care about lock/unlock
        : base_t(copy_from.clock())
    {
    }

    template <class ForeignAllocator, class ForeignTraits>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator, ForeignTraits>& copy_from)
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

template<class CharT, bool null_terminated = true,
         class Traits = std::char_traits<typename estd::remove_const<CharT>::type >,
         class StringTraits = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_traits<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_traits<Traits, int16_t, estd::is_const<CharT>::value> >::type>
class basic_string
        : public estd::basic_string<
                CharT, Traits,
                estd::internal::single_fixedbuf_runtimesize_allocator < CharT, null_terminated >,
                StringTraits>
{
    typedef estd::basic_string<
            CharT, Traits,
            estd::internal::single_fixedbuf_runtimesize_allocator < CharT, null_terminated >,
            StringTraits>
            base_t;

    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::impl_type helper_type;

protected:
    typedef typename base_t::size_type size_type;
    typedef typename allocator_type::InitParam init_t;

    // certain varieties (such as basic_string_view and layer3::const_string) only have one size, the initial
    // buffer size
    basic_string(CharT* buffer, size_type buffer_size, bool) :
        base_t(init_t(buffer, buffer_size))
    {
    }

public:
    /*
    template <size_type N>
    basic_string(const CharT (&buffer) [N], bool source_null_terminated = true) :
        base_t(typename allocator_type::InitParam(buffer, N))
    {
        base_t::helper.size(N - (source_null_terminated ? 1 : 0));
    } */

    template <size_type N>
    basic_string(CharT (&buffer) [N], size_type initial_size = -1) :
        base_t(init_t(buffer, N))
    {
        if(initial_size == -1)
            initial_size = strlen(buffer);

        // TODO: Enable for a string-mode version
        //base_t::ensure_total_size(initial_size);
        base_t::impl().size(initial_size);
    }

    basic_string(CharT* buffer, size_type initial_size, size_type buffer_size) :
        base_t(init_t(buffer, buffer_size))
    {
        base_t::impl().size(initial_size);
    }

    template <class ForeignAllocator, class ForeignTraits>
    basic_string(const estd::basic_string<CharT, Traits, ForeignAllocator, ForeignTraits> & copy_from)
        // FIX: very bad -- don't leave things locked!
        // only doing this because we often pass around layer1, layer2, layer3 strings who
        // don't care about lock/unlock
        : base_t(init_t(copy_from.clock(), copy_from.max_size()))
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
        if(!impl_type::uses_termination())
        {
            base_t::helper.size(copy_from.size());
        } */
    }


    template <class ForeignAllocator, class ForeignTraits>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator, ForeignTraits>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


// defaults to null-terminated variety
typedef basic_string<char> string;

// Non-NULL-terminated const strings use layer3
// NULL-terminated const strings use layer2
// TODO: Remake this into basic_string_view
class const_string : public basic_string<const char, false>
{
    typedef basic_string<const char, false> base_t;
    typedef typename base_t::size_type size_type;

public:
    const_string(const char* s, size_type len) :
        base_t(s, len, true) {}


    template <size_type N>
    const_string(const char (&buffer) [N], bool source_null_terminated = true) :
        base_t(buffer, source_null_terminated ? strlen(buffer) : N, true) {}

/*
    template <size_type N>
    const_string(const char (&buffer) [N], bool source_null_terminated = true) :
        base_t(buffer, source_null_terminated) {} */
};


}






template <class CharT, class Traits, class Alloc>
bool operator ==( const CharT* lhs, const basic_string<CharT, Traits, Alloc>& rhs)
{
    return rhs.compare(lhs) == 0;
}

/*
template <class Traits, class Alloc>
bool operator ==( const basic_string<typename Traits::char_type, Traits, Alloc>& lhs,
                  const typename Traits::char_type* rhs)
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
} */

template <class CharT, class StringTraits, class Alloc>
bool operator ==( const basic_string<CharT, typename StringTraits::char_traits, Alloc, StringTraits>& lhs,
                  typename estd::add_const<CharT>::type* rhs)
{
    return lhs.compare(rhs) == 0;
}



// have to do this because of our special const char stuff
// initial tests doing std::char_traits<const char> seem to work, but I don't
// trust them - where do they specialize to?
template <class TCharLeft,
          class TCharRight,
          class StringTraitsLeft,
          class StringTraitsRight,
          class AllocLeft, class AllocRight>
bool operator ==( const basic_string<TCharLeft, typename StringTraitsLeft::char_traits, AllocLeft, StringTraitsLeft>& lhs,
                  const basic_string<TCharRight, typename StringTraitsRight::char_traits, AllocRight, StringTraitsRight>& rhs)
{
    return lhs.compare(rhs) == 0;
}

}


#include "internal/to_string.h"

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
template <class CharT, class Traits, class Allocator>
inline std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
               const estd::basic_string<CharT, typename Traits::char_traits, Allocator, Traits>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.fake_const_lock(), str.size());

    str.cunlock();

    return os;
}


// FIX: Temporarily using this, but really managing interaction between two sets
// of traits is a better way to go, ala:
// template <class CharT, class Traits, class Allocator,
//  class OStreamTraits = std::char_traits<remove_const<Traits::char_type>>>
template <class CharT, class Traits, class Allocator>
inline std::ostream&
    operator<<(std::ostream& os,
               const estd::basic_string<const CharT, typename Traits::char_traits, Allocator, Traits>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
}
#endif

