#pragma once

namespace estd {


namespace internal {

// due to FEATURE_CPP_ENUM_CLASS not available everywhere
struct encodings
{
    enum values
    {
        ASCII,
        ISO8859_1,
        UTF8,
        UTF16
    };
};


}


}