#pragma once

#include "../cstdint.h"
#include "../traits/char_traits.h"

namespace estd { namespace internal {


// DEBT: Works OK, but will get confused if names start with the same letters
template <class TChar>
class chooser
{
    typedef TChar char_type;

    int chosen_ = -1;
    unsigned i = 0;

public:
    int chosen() const { return chosen_; }

    void reset()
    {
        chosen_ = -1;
        i = 0;
    }


    static inline std::size_t get_size(const char* str)
    {
        return estd::char_traits<char>::length(str);
    }


    template <class TContainer>
    static inline std::size_t get_size(const TContainer& container)
    {
        return container.size();
    }

    template <class TContainer, std::size_t N>
    inline bool process(const TContainer (&containers)[N], const char_type c)
    {
        // Look through all the containers to try to find the first match
        if(chosen_ == -1)
        {
            for (unsigned j = 0; j < N; ++j)
            {
                const TContainer& container = containers[j];

                if(container[i] == c)
                {
                    chosen_ = j;
                    break;
                }
            }
        }
        else
        {
            // DEBT: Consider doing this with a pointer instead
            const TContainer& container = containers[chosen_];

            if(i == get_size(container) - 1)
                return true;
            else if(container[i] != c)
            {
                chosen_ = -1;
                return true;
            }
        }

        ++i;
        return false;
    }

    template <class TContainer, std::size_t N, typename TIter>
    int choose2(const TContainer (&containers)[N], TIter& in, TIter end, bool autoReset = false) { return chosen_; }

    template <class TContainer, std::size_t N, typename TIter>
    int choose(const TContainer (&containers)[N], TIter& in, TIter end, bool reset)
    {
        if(reset) this->reset();

        for(; in != end; ++in)
        {
            if(process(containers, *in)) return chosen_;
        }

        return chosen_ = -1;
    }

    /*
    template <class TContainer, std::size_t N, typename TIter>
    static int choose(const TContainer (&containers)[N], TIter& in, TIter end)
    {
        chooser
    } */
};

}}