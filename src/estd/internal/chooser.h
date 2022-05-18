#pragma once

#include "../cstdint.h"
#include "../traits/char_traits.h"

namespace estd { namespace internal {


// DEBT: Works OK, but will get confused if names start with the same letters
class chooser
{
    short chosen_;
    unsigned short i;

public:
    chooser() : chosen_(-1), i(0) {}

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

    // We do demand random access array-style containers.  Otherwise, we'd have to maintain
    // a whole list of iterators, one per container
    template <class TContainer, std::size_t N, typename value_type>
    inline bool process(const TContainer (&containers)[N], const value_type c)
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

            // TODO: Optimize, detect containers which require processing to deduce size.
            // Likely this is most easily done in reverse, to tag containers which hold
            // a size variable/constant and detect that
            const unsigned sz = get_size(container);

            if(i == sz - 1)
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
    TIter choose(const TContainer (&containers)[N], TIter in, TIter end, bool reset)
    {
        if(reset) this->reset();

        for(; in != end; ++in)
        {
            // On a match, be sure to bump up iterator by one since we match on the last
            // character but C++ convention tends to want iterator one past the match
            if(process(containers, *in)) return ++in;
        }

        chosen_ = -1;
        return in;
    }

    template <class TContainer, std::size_t N, typename TIter>
    static inline int choose(const TContainer (&containers)[N], TIter& in, TIter end)
    {
        chooser c;
        in = c.choose(containers, in, end, false);
        return c.chosen();
    }

    template <class TContainer, std::size_t N>
    static inline int choose(const TContainer (&containers)[N], TContainer& c)
    {
        return choose(containers, c, c + get_size(c));
    }
};

}}