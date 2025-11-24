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
};


template <class T, class Traits = void>
struct rtto : rtto_base
{
    using value_type = T;
    using pointer = T*;
    using traits = Traits;

    static inline int copy(pointer from, void* to, std::true_type)
    {
        new (to) value_type(*from);
        return 0;
    }

    static inline int create(void* storage, std::true_type)
    {
        new (storage) value_type;
        return 0;
    }

    static ESTD_CPP_CONSTEVAL int copy(void*, void*, std::false_type) { return EINVAL; }
    static ESTD_CPP_CONSTEVAL int create(void* storage, std::false_type) { return EINVAL; }

    static int utility(modes mode, void* p0, int p1, void* p2)
    {
        auto this_ = static_cast<pointer>(p0);

        switch(mode)
        {
            case COPY:
                // FIX: estd flavor isn't resolving down to estd::true_type/estd::false_type
                //return copy(this_, p2, std::is_copy_constructible<value_type>{});
                return copy(this_, p2, std::true_type{});

            case MOVE:
                new (p2) value_type(std::move(*this_));
                break;

            case MOVE_AND_DESTROY:
                new (p2) value_type(std::move(*this_));
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
                return create(p0, std::is_trivially_constructible<value_type>{});
                //return create(p0, bool_constant<false>{});
#endif

            default:
                return ENOSYS;
        }

        return 0;
    }
};


}}
