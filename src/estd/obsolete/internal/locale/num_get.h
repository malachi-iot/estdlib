#if UNUSED
// this code went into old estd::num_get::helper
// It predates the iterated::num_get code


        // DEBT: We can likely place this elsewhere since it doesn't actually need 'this'
        //       though it being locale-bound may affect that down the line
        // NOTE: legacy version doesn't use state machine.  Due to our 'good' variable, I'd
        // rate our state machine as equally efficient as legacy version, but more powerful.
        // Given we can dogfood and reuse with apparently no penalty, we do it
        template <unsigned base, class T>
        static iter_type get_unsigned_integer_legacy(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            // DEBT: Consider using use_facet, though really not necessary at this time
            typedef cbase<char_type, base, locale_type> cbase_type;
            //use_facet<cbase<char_type, base> >(str.getloc()).from_char(*i);

            v = 0;
            // Since we are using forward-only iterators, we can't retain an
            // old 'i' to compare against
            bool good = false;

            for(; i != end; ++i, good = true)
            {
                typename cbase_type::optional_type n = cbase_type::from_char(*i);

                if(n.has_value())
                {
                    /**
                     * "If the conversion function fails to convert the entire field, the value 0
                     *  is stored in v
                     *
                     *  If the type of v is a signed integer type and the conversion
                     *  function results in a positive or negative value too large to fit in it,
                     *  the most positive or negative representable value is stored in v, respectively
                     *
                     *  If the type of v is an unsigned integer type and the conversion function results
                     *  in a value that does not fit in it, the most positive representable value is
                     *  stored in v." [1]
                     *
                     *  Beware that raise_and_add *might* clobber v in that condition.
                     *  See documentation for that function
                     *
                     *  "In any case, if the conversion function fails std::ios_base::failbit is
                     *   assigned to err" [1]
                     *
                     *  This *strongly implies* that the capped conversion is not considered a failure.
                     *  Something feels wrong about letting an overflow pass by unnoticed, so I am
                     *  setting the fail bit here despite what they imply
                     */
                    if(!estd::internal::raise_and_add(v, base, n.value()))
                    {
                        // TODO: It's subtle, but spec expects us to read all the digits out of
                        // the input iterator, even on overflow

                        err |= ios_base::failbit;
                        return i;
                    }
                }
                else
                {
                    if(!good)
                        err |= ios_base::failbit;

                    return i;
                }
            }

            err |= ios_base::eofbit;
            return i;
        }

        template <unsigned base, class T>
        static iter_type get_signed_integer_legacy(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            bool negative = false;

            // DEBT: We could possibly constexpr up and down the facet/locale chain which may
            // more readily optimize this line.  That said, chances are it's already optimized
            // to the max (i.e. hypen = '-')
            // NOTE: spec strongly implies hyphen is *always* used to denote a negative number
            // regardless of locale
            const char hyphen =
                use_facet<ctype<char_type> >(str.getloc()).widen('-');

            // TODO: Might be able to merely copy this iterator and to this evaluation
            // at the end.  Perhaps do a specialization for this based on policy
            if(*i == hyphen)
            {
                negative = true;
                ++i;
            }

            i = get_unsigned_integer<base>(i, end, err, str, v);

            if(negative) v = -v;

            return i;
        }


    // Lifted from
    // https://stackoverflow.com/questions/221001/performance-question-fastest-way-to-convert-hexadecimal-char-to-its-number-valu
    template <class T>
    void get_integer_ascii_hexadecimal_unchecked(iter_type i, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        v = 0;

        // Doesn't detect errors

        for(; i < (end - 1); ++i)
        {
            char_type c = *i;

            c|=0x20;

            c = c<='9'? c+0xD0 : c+0xA9;

            v <<= 4;
            v += c;
        }

        err |= ios_base::eofbit;
    }




        static iter_type get_bool_legacy(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, bool& v)
        {
            if(str.flags() & ios_base::boolalpha)
            {
                numpunct<char_type, locale_type> np =
                        use_facet<numpunct<char_type> >(str.getloc());

                // tempted to get algorithmically fancy here, but with only two things to
                // compare, brute force makes sense
                estd::layer2::basic_string<const char_type, 0> names[]
                #ifdef FEATURE_CPP_INITIALIZER_LIST
                {
                    np.truename(),
                    np.falsename()
                };
                #else
                ;
                names[0] = np.truename();
                names[1] = np.falsename();
                #endif

                int chosen = estd::internal::chooser::choose(names, in, end);

                if(in == end)
                    err |= ios_base::eofbit;

                if(chosen == -1)
                {
                    v = false;
                    err |= ios_base::failbit;
                }
                else
                    v = chosen == 0;
            }
            else
            {
                unsigned temp;
                in = get_unsigned_integer<2>(in, end, err, str, temp);
                // DEBT: Try to avoid using temporary.
                // No bounds check necessary here, since specifying base 2 already does that
                v = temp == 1;
            }
            return in;
        }


#endif