#pragma once

#include "../system_error.h"

#include "type_traits.h"
#include "utility.h"

namespace estd { namespace internal {

struct rtto_base
{
    enum modes
    {
        COPY,
        MOVE,
        MOVE_AND_DESTROY,
        DELETE,
        SIZE,

        // EXPERIMENTAL
        CREATE
    };

    using utility_type = int (*)(modes, void*, int, void*);

    class utility_base
    {
        utility_type u_;

        /*
         * FIX: Causes a recursion stack overflow
        ~utility_base()
        {
            if(u_)  u_(DELETE, this, nullptr);
        }   */

    public:
        ESTD_CPP_DEFAULT_RULE_OF_5(utility_base)
        constexpr explicit utility_base(utility_type u) :
            u_{u}
        {}

        // EXPERIMENTAL
        int create(void* dest, int sz = 0)
        {
            return u_(CREATE, this, sz, 0);
        }

        int copy_to(void* dest, int sz = 0) const
        {
            return u_(COPY, const_cast<utility_base*>(this), sz, dest);
        }

        int move_to(void* dest, int sz = 0)
        {
            return u_(MOVE, this, sz, dest);
        }

        int move_to_and_destroy(void* dest, int sz = 0)
        {
            return u_(MOVE_AND_DESTROY, this, sz, dest);
        }

        void destroy()
        {
            u_(DELETE, this, 0, nullptr);
        }

        int size() const
        {
            return u_(SIZE, nullptr, 0, nullptr);
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
        virtual int copy_to(void* src, void* dst) = 0;
        virtual int move_to(void* src, void* dst) = 0;
    };
};


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

    static inline int copy(pointer from, void* to, std::true_type)
    {
        new (to) value_type(*from);
        return 0;
    }

    static inline int move(pointer from, void* to, std::true_type)
    {
        new (to) value_type(std::move(*from));
        return 0;
    }

    static inline int create(void* storage, std::true_type)
    {
        new (storage) value_type;
        return 0;
    }

    static ESTD_CPP_CONSTEVAL int copy(void*, void*, std::false_type) { return EINVAL; }
    static ESTD_CPP_CONSTEVAL int move(void*, void*, std::false_type) { return EINVAL; }
    static ESTD_CPP_CONSTEVAL int create(void*, std::false_type) { return EINVAL; }

    constexpr static bool size_ok(int sz)
    {
        return sz == 0 || sz >= value_sz;
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
                if(!size_ok(p1)) return ENOMEM;
                // FIX: estd flavor isn't resolving down to estd::true_type/estd::false_type
                //return copy(this_, p2, estd::is_copy_constructible<value_type>{});
                return copy(this_, p2, is_copy_constructible{});

            case MOVE:
                return move(this_, p2, p1);

            case MOVE_AND_DESTROY:
                if(!size_ok(p1)) return ENOMEM;
                if(move(this_, p2, is_move_constructible{}) != 0) return EINVAL;
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
        int copy_to(void* src, void* dest) override
        {
            return copy(src, dest, is_constructible{});
        }

        int move_to(void* src, void* dest) override
        {
            return move(src, dest, is_constructible{});
        }
    };
};


}}
