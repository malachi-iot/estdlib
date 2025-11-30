#pragma once

#include "../system_error.h"

#include "type_traits.h"
#include "utility.h"

// EXPERIMENTAL still and occupies ROM space even when not used
#ifndef FEATURE_ESTD_RTTO_GET_METADATA
#define FEATURE_ESTD_RTTO_GET_METADATA 0
#endif

namespace estd { namespace internal {

struct rtto_modes
{
    enum modes
    {
        COPY,
        MOVE,
        MOVE_AND_DESTROY,
        DELETE,
        SIZE,

        // +++ EXPERIMENTAL
        CREATE,
        GET_METADATA,
        COPY_AND_SWAP,
        // --- EXPERIMENTAL
    };

    using utility_type = int (*)(modes, void*, int, void*);
};



constexpr int copy_to(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::COPY, src, sz, dest);
}

constexpr int move_to(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::MOVE, src, sz, dest);
}

constexpr int move_to_and_destroy(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::MOVE_AND_DESTROY, src, sz, dest);
}

inline ESTD_CPP_CONSTEXPR(14) void destroy(rtto_modes::utility_type u, void* src)
{
    u(rtto_modes::DELETE, src, 0, nullptr);
}



struct rtto_base : rtto_modes
{
    // EXPERIMENTAL
    struct metadata
    {
        const int value_sz;
        bool copyable : 1;
        bool moveable : 1;
    };

    class base
    {
        using this_type = base;

        utility_type u_;

        /*
         * FIX: Causes a recursion stack overflow
        ~utility_base()
        {
            if(u_)  u_(DELETE, this, nullptr);
        }   */

    public:
        ESTD_CPP_DEFAULT_RULE_OF_5(base)
        constexpr explicit base(utility_type u) :
            u_{u}
        {}

        // EXPERIMENTAL
        int create(void* dest, int sz = 0)
        {
            return u_(CREATE, this, sz, 0);
        }

        constexpr int copy_to(void* dest, int sz = 0) const
        {
            return internal::copy_to(u_, const_cast<this_type*>(this), dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) int move_to(void* dest, int sz = 0)
        {
            return internal::move_to(u_, this, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) int move_to_and_destroy(void* dest, int sz = 0)
        {
            return internal::move_to_and_destroy(u_, this, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) void destroy()
        {
            internal::destroy(u_, this);
        }

        int size() const
        {
            return u_(SIZE, nullptr, 0, nullptr);
        }

        int get_metadata(const metadata** out)
        {
            return u_(GET_METADATA, nullptr, 0, out);
        }
    };

    class virtual_base
    {
    public:
        virtual ~virtual_base() = default;

        virtual int copy_to(void*, int = 0) = 0;
        virtual int move_to(void*, int = 0) = 0;

        void destroy() { this->~virtual_base(); }
    };

    // UNFINISHED, EXPERIMENTAL
    // Seems a fully polymorphic approach *may* be superior to the function ptr dispatch
    // method.  Sussing that out
    class virt
    {
        virtual int copy_to(void* src, void* dst) const = 0;
        virtual int move_to(void* src, void* dst) const = 0;
        virtual void destroy(void*) const = 0;
        virtual int size() const = 0;
    };
};


// TODO: Look into https://en.cppreference.com/w/cpp/memory/polymorphic_allocator.html to
// see if there's any overlap.  So far it doesn't seem so
template <class T, class Traits = void>
struct rtto : rtto_base
{
    using value_type = T;
    using pointer = T*;
    using traits = Traits;

    // DEBT: Switch these to estd variety for AVR compability
    using is_copy_constructible = std::is_copy_constructible<value_type>;
    using is_move_constructible = std::is_move_constructible<value_type>;
    using is_trivially_constructible = std::is_trivially_constructible<value_type>;
    using is_constructible = std::is_default_constructible<value_type>;

    static constexpr int value_sz = sizeof(value_type);

    // EXPERIMENTAL
    //static constexpr metadata mdata{value_sz};

    static ESTD_CPP_CONSTEXPR(14) int copy(pointer from, void* to, std::true_type)
    {
        new (to) value_type(*from);
        return 0;
    }

    static ESTD_CPP_CONSTEXPR(14) int move(pointer from, void* to, std::true_type)
    {
        new (to) value_type(std::move(*from));
        return 0;
    }

    static ESTD_CPP_CONSTEXPR(14) int create(void* storage, std::true_type)
    {
        new (storage) value_type;
        return 0;
    }

    static constexpr int copy(void*, void*, std::false_type) { return EINVAL; }
    static constexpr int move(void*, void*, std::false_type) { return EINVAL; }
    static constexpr int create(void*, std::false_type) { return EINVAL; }

    constexpr static bool size_ok(int sz)
    {
        return sz == 0 || sz >= value_sz;
    }

    static inline int copy(pointer from, void *to, int sz)
    {
        if(!size_ok(sz)) return ENOMEM;

        return copy(from, to, is_copy_constructible{});
    }

    static inline int move(pointer from, void *to, int sz)
    {
        if(!size_ok(sz)) return ENOMEM;

        return move(from, to, is_move_constructible{});
    }

    static int utility(modes mode, void* p0, int p1, void* p2)
    {
        auto this_ = static_cast<pointer>(p0);

        switch(mode)
        {
            case COPY:
                // FIX: estd flavor isn't resolving down to estd::true_type/estd::false_type
                //return copy(this_, p2, estd::is_copy_constructible<value_type>{});
                return copy(this_, p2, p1);

            case MOVE:
                return move(this_, p2, p1);

            case MOVE_AND_DESTROY:
                if(int rc = move(this_, p2, p1) != 0) return rc;
                ESTD_CPP_ATTR_FALLTHROUGH;

            case DELETE:
                this_->~value_type();
                break;

            case SIZE:
                return sizeof(value_type);

#if !defined(__AVR__)
            // EXPERIMENTAL
            case CREATE:
                // FIX: estd is_trivially_constructible falls apart here
                return create(p0, is_constructible{});
                //return create(p0, bool_constant<false>{});
#endif

#if FEATURE_ESTD_RTTO_GET_METADATA
            // EXPERIMENTAL
            case GET_METADATA:
            {
                // DEBT: Incorrect convention because Qt Creator loses it's f***in mind here
                static constexpr const metadata mdata{
                    value_sz, is_copy_constructible::value, is_move_constructible::value
                };
                auto dest = static_cast<const metadata**>(p2);
                *dest = &mdata;
                break;
            }
#endif

            // EXPERIMENTAL
            case COPY_AND_SWAP:
            {
                break;
            }

            default:
                return ENOSYS;
        }

        return 0;
    }

    // UNFINISHED, EXPERIMENTAL
    // Seems a fully polymorphic approach *may* be superior to the function ptr dispatch
    // method.  Sussing that out
    // While convenient, we do sling the 'this' pointer around for only indirect gains.
    // Possibly an inefficiency as compared to fnptr
    class virt : public rtto_base::virt
    {
    public:
        int copy_to(void* src, void* dest) const override
        {
            return copy(src, dest, is_constructible{});
        }

        int move_to(void* src, void* dest) const override
        {
            return move(src, dest, is_constructible{});
        }

        void destroy(void*) const override
        {
            // TBD
        }

        int size() const override { return value_sz; }
    };
};


}}
