#pragma once

namespace estd {

struct unexpect_t
{
#if __cplusplus >= 201103L
    explicit unexpect_t() = default;
#endif
};

}