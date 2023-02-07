#pragma once

#include <estd/vector.h>
#include <estd/functional.h>

struct FunctionList
{
    typedef estd::detail::impl::function_fnptr1<void()> impl_type;
    estd::layer1::vector<impl_type::copyable_model<128>, 10> list;

    void invoke()
    {
        for(auto& d : list)
        {
            d._exec();
        }
    }
};
