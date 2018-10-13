#pragma once

#include "memory.h"
#include "allocators/fixed.h"
#include "internal/dynamic_array.h"
#include "traits/char_traits.h"
#include "traits/string.h"
#include "policy/string.h"
#include "port/string.h"
#include "algorithm.h"
#include "span.h"
// NOTE: Dormant, always using old-style stdlib.h for now
#ifdef FEATURE_STD_CSDLIB
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE
#include <ostream>
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
#ifdef FEATURE_STD_MEMORY
    class Allocator = std::allocator<CharT>,
    class StringPolicy = experimental::sized_string_policy<Traits>
#else
    class Allocator, class StringPolicy
#endif
> class basic_string :
        public internal::dynamic_array<internal::impl::dynamic_array<Allocator, StringPolicy> >
{
    typedef internal::dynamic_array<internal::impl::dynamic_array<Allocator, StringPolicy> > base_t;

    /*
     * Was using this for Arduino port += but actually it's not necessary and causes linker
     * errors for POSIX mode
    template <class TInput>
    friend basic_string operator +=(basic_string& lhs, TInput rhs); */

public:
    typedef typename base_t::size_type size_type;

protected:
    template <class THelperParam>
    basic_string(const THelperParam& p) : base_t(p) {}

public:
    basic_string() {}

    template <class TImpl>
    basic_string(const internal::allocated_array<TImpl>& copy_from) :
        base_t(copy_from) {}

    typedef CharT value_type;
    typedef Traits traits_type;
    typedef typename base_t::allocator_type allocator_type;
    typedef StringPolicy policy_type;

    typedef typename allocator_type::handle_type handle_type;

    static CONSTEXPR size_type npos = -1;

    size_type length() const { return base_t::size(); }

    basic_string& erase(size_type index = 0, size_type count = -1)
    {
        size_type size_minus_index = base_t::size() - index;
        // NOTE: A bit tricky, if we don't use helper size_minus_index, template
        // resolution fails, presumably because the math operation implicitly
        // creates an int
        size_type to_remove_count = estd::min(count, size_minus_index);

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

    basic_string& append(const value_type* s, size_type count)
    {
        base_t::append(s, count);
        return *this;
    }


    basic_string& append(const value_type* s)
    {
        return append(s, strlen(s));
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


    /*
    template <class TString>
    basic_string& operator += (TString s)
    {
        return append(s);
        return *this;
    } */


    basic_string& operator += (value_type c)
    {
        base_t::push_back(c);
        return *this;
    }


    template <class TForeignImpl>
    basic_string& operator=(const internal::allocated_array<TForeignImpl>& copy_from)
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



    // Keeping this as I expect to eventually need a string/char traits aware
    // version of the character-by-character comparison
    template <class TImpl>
    bool starts_with(const internal::allocated_array<TImpl>& compare_to) const
    {
        return base_t::starts_with(compare_to);
    }
};


// this typedef relies on std::allocator
#ifdef FEATURE_STD_MEMORY
typedef basic_string<char> string;
#endif



namespace layer1 {


template<class CharT, size_t N, bool null_terminated = true, class Traits = std::char_traits<CharT >,
        class StringPolicy = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_policy<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_policy<Traits, int16_t, estd::is_const<CharT>::value> >::type
                >
class basic_string
        : public estd::basic_string<
                CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer1::allocator<CharT, N>,
#else
                estd::internal::single_fixedbuf_allocator <CharT, N>,
#endif
                StringPolicy>
{
    typedef estd::basic_string<
                CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer1::allocator<CharT, N>,
#else
                estd::internal::single_fixedbuf_allocator <CharT, N>,
#endif
                StringPolicy>
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
    }

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

    // layer1 strings can safely issue a lock like this, since unlock is a no-op
    CharT* data() { return base_t::lock(); }

    const CharT* data() const { return base_t::clock(); }

    operator basic_string_view<CharT, Traits>() const
    {
        return basic_string_view<CharT, Traits>(data(), base_t::size());
    }
};


#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;
#else
template <size_t N, bool null_terminated = true>
class string : public basic_string<char, N, null_terminated>
{
    typedef basic_string<char, N> base_t;

public:
    string() {}

    string(const char* s) : base_t(s)
    {
    }
};
#endif

}


// TODO: refactor layer2 & layer3 to utilize a handle and not a CharT*
namespace layer2 {

template<class CharT, size_t N, bool null_terminated = true,
         class Traits = std::char_traits<typename estd::remove_const<CharT>::type >,
         class StringPolicy = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_policy<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_policy<Traits, int16_t, estd::is_const<CharT>::value> >::type >
class basic_string
        : public estd::basic_string<
                CharT,
                Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer2::allocator<CharT, N>,
#else
                estd::internal::single_fixedbuf_allocator < CharT, N, CharT* >,
#endif
                StringPolicy >
{
    typedef estd::basic_string<
            CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
            estd::layer2::allocator<CharT, N>,
#else
            estd::internal::single_fixedbuf_allocator < CharT, N, CharT* >,
#endif
            StringPolicy >
            base_t;
    typedef typename base_t::impl_type helper_type;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::size_type size_type;

    // this one we presume we're looking at either:
    // - an already initialized null terminated string
    // - a size=capacity variant, in which str_buffer isn't (necessarily) null terminated
    //   but size() still reflects the right size of the string
    // This particular constructor is good for string literals, assuming CharT is const char
    basic_string(CharT* str_buffer) : base_t(str_buffer)
    {
    }

    // n means assign length to n, ignoring any null termination if present
    basic_string(CharT* str_buffer, int n) : base_t(str_buffer)
    {
        // doing this separately from above constructor because not all
        // specializations permit explicitly (re)sizing the string
        base_t::impl().size(n);
    }

    // See 'n' documentation above
    // FIX: above constructor greedily consumes this one's chance at running.
    // Before, I was using const CharT* to differenciate it but technically
    // a const CharT* is just incorrect as the underlying layer2::basic_string
    // isn't intrinsically const
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

    basic_string& operator=(const CharT* s)
    {
        //return base_t::operator =(s);
        base_t::assign(s, strlen(s));
        return *this;
    }

    // layer2 strings can safely issue a lock like this, since unlock is a no-op
    CharT* data() { return base_t::lock(); }

    const CharT* data() const { return base_t::clock(); }

    // A little clumsy since basic_string_view treats everything as const already,
    // so if we are converting from a const_string we have to remove const from CharT
    operator basic_string_view<typename estd::remove_const<CharT>::type, Traits>() const
    {
        return basic_string_view<typename estd::remove_const<CharT>::type, Traits>(data(), base_t::size());
    }
};


#ifdef FEATURE_CPP_ALIASTEMPLATE
template <size_t N = 0, bool null_terminated = true>
using string = basic_string<char, N, null_terminated>;
#endif

typedef basic_string<const char, 0> const_string;

namespace experimental {

template <size_t N>
basic_string<char, N, true> make_string(char (&buffer)[N], int n = -1)
{
    return basic_string<char, N, true>(buffer, n);
}

}

}

namespace layer3 {

template<class CharT, bool null_terminated = true,
         class Traits = std::char_traits<typename estd::remove_const<CharT>::type >,
         class Policy = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_policy<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_policy<Traits, int16_t, estd::is_const<CharT>::value> >::type>
class basic_string
        : public estd::basic_string<
                CharT, Traits,
// FIX: Not ready yet, because layer3::allocator constructor and class InitParam doesn't fully
// initialize underlying allocator
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer3::allocator<CharT, typename Policy::size_type>,
#else
                estd::internal::single_fixedbuf_runtimesize_allocator < CharT >,
#endif
                Policy>
{
    typedef estd::basic_string<
            CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
            estd::layer3::allocator<CharT, typename Policy::size_type>,
#else
            estd::internal::single_fixedbuf_runtimesize_allocator < CharT >,
#endif
            Policy>
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
// NOTE: Remake this into basic_string_view.  Though the basic_string_view has an additional variable (potentially)
//       for tracking the truncations
// If the above does not happen, then consider promoting/typedefing this const_string to the regular
// estd:: namespace too
class const_string : public basic_string<const char, false>
{
    typedef basic_string<const char, false> base_t;
    typedef base_t::size_type size_type;

public:
    const_string(const char* s, size_type len) :
        base_t(s, len, true) {}


    template <size_type N>
    const_string(const char (&buffer) [N], bool source_null_terminated = true) :
        base_t(buffer, source_null_terminated ? strlen(buffer) : N, true) {}

    // convenience method since std::vector and std::string itself are reported to convert
    // uneventfully between unsigned char and char
    // note it's a little bit bad because of the distant possibility of a
    // byte != unsigned char != uint8_t
    const_string(const estd::const_buffer& cast_from) :
        base_t(reinterpret_cast<const char*>(cast_from.data()), cast_from.size(), true) {}
};


}


template< class CharT, class Traits, class Alloc, class Policy, class TString >
    estd::basic_string<CharT,Traits,Alloc,Policy>&
        operator+=(estd::basic_string<CharT,Traits,Alloc,Policy>& lhs,
                   TString rhs )
{
    lhs.append(rhs);
    return lhs;
}



template <class CharT, class Traits, class Alloc, class TStringTraits>
bool operator ==( const CharT* lhs, const basic_string<CharT, Traits, Alloc, TStringTraits>& rhs)
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


// FIX: This doesn't account for conversion errors, but should.  std version
// throws exceptions
template <class TChar, class Traits, class Alloc, class TStringTraits>
long stol(
        const basic_string<TChar, Traits, Alloc, TStringTraits>& str,
        size_t pos = 0, int base = 10)
{
    // FIX: very clunky way to ensure we're looking at a null terminated string
    // somehow I never expose null_termination indicators on the string itself
    TStringTraits::is_null_termination(0);

    const TChar* data = str.clock();
    typename estd::remove_const<TChar>::type* end;
    long result = strtol(data, &end, base);
    str.cunlock();
    return result;
}

}


#include "internal/to_string.h"

namespace estd {
namespace internal {

// FIX: Is not helping, we still get warnings.  Might be because string_convert
// is picked up before this specialization, but I don't think so - that's not
// usually something specializations are subject to
template<>
inline CONSTEXPR uint8_t maxStringLength<estd::internal::no_max_string_length_tag>()
{ return 0; }

}
}

#ifdef FEATURE_ESTD_IOSTREAM_NATIVE

//A bit finicky so that we can remove const (via Traits::char_type)
template <class Allocator, class StringTraits,
          class Traits,
          class CharT>
inline std::basic_ostream<typename Traits::char_type, Traits>&
    operator<<(std::basic_ostream<typename Traits::char_type, Traits>& os,
               const estd::basic_string<CharT, Traits, Allocator, StringTraits>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
}

/*
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
} */
#endif




