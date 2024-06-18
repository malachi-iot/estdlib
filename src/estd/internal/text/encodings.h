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

// Can ASCII be upcast (or 1:1?) to specified encoding?
constexpr bool is_ascii_compatible(encodings::values e)
{
    return e == encodings::ASCII ||
        e == encodings::ISO8859_1 ||
        e == encodings::UTF8 ||
        e == encodings::UTF16;
}


}


}