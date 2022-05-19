#pragma once

#include "../array.h"

namespace estd { namespace experimental {

struct nibble
{
    uint8_t value;
};

struct nibble_holder
{
    typedef experimental::nibble value_type;

    uint8_t* d;
    bool toggle;

    nibble_holder(uint8_t* d, bool toggle) : d(d), toggle(toggle) {}

    operator value_type() const
    {
        uint8_t v = *d;
        if(toggle)
        {
            v &= 0xf;
            return value_type{v};
        }
        else
        {
            v >>= 4;
            return value_type{v};
        }
    }

    inline bool equals(uint8_t v)
    {
        if(toggle)
        {
            uint8_t _d = *d & 0xf;
            return _d == v;
        }
        else
        {
            uint8_t _d = *d >> 4;
            return _d == v;
        }
    }


    inline nibble_holder& operator =(uint8_t v)
    {
        if(toggle)
        {
            *d &= 0xf0;
            *d |= v;
        }
        else
        {
            *d &= 0xf;
            *d |= v << 4;
        }
        return *this;
    }

protected:
    nibble_holder() : d(NULLPTR) {}
};




}

template<std::size_t N>
struct array<experimental::nibble, N>
{
    typedef unsigned short size_type;
    typedef experimental::nibble value_type;
    typedef experimental::nibble_holder holder_type;

    uint8_t data_[(1 + N) / 2];



    struct iterator : holder_type
    {
        iterator(uint8_t* d, bool toggle) : holder_type(d, toggle) {}

        /*
        value_type operator *() const
        {
            return experimental::nibble_holder::operator value_type();
        } */

        const holder_type& operator*() const { return *this; }

        // prefix
        iterator& operator++()
        {
            if(!(toggle = !toggle))
                ++d;

            return *this;
        }

        // prefix
        iterator& operator--()
        {
            if((toggle = !toggle))
                --d;

            return *this;
        }
    };

    iterator begin() { return iterator(data_, 0 % 2); }

    iterator end() { return iterator(data_ + (N / 2), N % 2); }

    /*
    value_type operator[](size_type pos)
    {
        bool mod = pos % 2;
        uint8_t val = data_[pos / 2];
        if(mod) val >> 4;
        return value_type{val};
    }
    */
    holder_type operator[](size_type pos)
    {
        bool mod = pos % 2;
        return holder_type(&data_[pos / 2], mod);
    }
};

}

namespace estd { namespace experimental {

inline bool operator==(estd::experimental::nibble_holder h, uint8_t v)
{
    return h.equals(v);
}



}}