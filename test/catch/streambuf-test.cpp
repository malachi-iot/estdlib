//
// Created by malachi on 4/29/22.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>
#include <estd/sstream.h>

using namespace estd;

using synthetic_streambuf_base = internal::impl::streambuf_base<estd::char_traits<char> >;

TEST_CASE("streambuf")
{
    const char raw_str[] = "raw 'traditional' output\n";
    constexpr int raw_str_len = sizeof(raw_str) - 1;

    SECTION("basic_streambuf test")
    {
        // NOTE: posix_streambuf probably technically should be able to take stdin AND stdout...
        //posix_streambuf<char> psb(*stdout);
        //internal::basic_streambuf_wrapped<posix_streambuf<char>& > sbw(psb);

        internal::basic_streambuf_wrapped<posix_streambuf<char> > sbw(*stdout);
        basic_streambuf<char>& sb = sbw;

#ifdef TEST_COUT
        sb.sputn(raw_str, sizeof(raw_str) - 1);
#endif
    }
    SECTION("impl::out_span_streambuf")
    {
        // FIX: Not quite there because span constructor won't switch between Extent and
        // non Extent version for explicit buffer just yet
        constexpr int val_size = 32;
        uint32_t val[val_size];
        uint8_t test1[] = { 0, 1, 2, 3 };

        val[0] = 0;

        typedef estd::internal::impl::out_span_streambuf<uint8_t> streambuf_impl_type;

        SECTION("initialization")
        {
            typedef estd::internal::streambuf<streambuf_impl_type> streambuf_type;

            SECTION("array init")
            {
                uint8_t buffer[32];

                streambuf_type streambuf(buffer);

                streambuf.sputn((uint8_t*)"abc", 3);

                REQUIRE(streambuf.pos() == 3);
                REQUIRE(buffer[0] == 'a');
            }
        }
        SECTION("full (non impl) version")
        {
            // 'test' actually sits dormant and does nothing.  artifact of previous test
            // approach
            estd::internal::impl::out_span_streambuf<uint8_t> test((uint8_t*)&val[0], 32);

            // NOTE: Wanted to do a ref version here but not sure if I actually
            // want that to be a supported technique
            // NOTE: This and many other of the '32' used around here are wrong, since
            // that's the uint32_t-oriented size
            estd::internal::streambuf<decltype(test)> sb((uint8_t*)&val[0], 32);

            sb.sputc(0);
            sb.sputc(1);
            sb.sputc(2);
            sb.sputc(3);

            // TODO: Make this endian-inspecific.  For the time being though I anticipate
            // these unit tests only running on x64 machines
            REQUIRE(val[0] == 0x03020100);

            int sz = sizeof(sb);

            // TODO: Re-enable this, upgraded out_span_streambuf and broke old sizing
            //REQUIRE(sz == sizeof(estd::span<uint32_t, 32>) + sizeof(size_t));

            //REQUIRE(sb.epptr() - sb.pbase() == 32 * 4);
        }
        SECTION("non-constexpr size version")
        {
            estd::internal::streambuf<streambuf_impl_type> sb((uint8_t*)&val[0], 32);

            sb.sputc(0);
            sb.sputc(1);
            sb.sputc(2);
            sb.sputc(3);

            // DEBT: Make this non-endian-specific
            REQUIRE(val[0] == 0x03020100);

            int sz = sizeof(sb);

            // TODO: Re-enable this, upgraded out_span_streambuf and broke old sizing
            // TODO: see why it grew from 24 to 32 bytes also (maybe because we inherit pos now
            // and before it was packing it? doubtful.  Probably because pos is this more complicated
            // pos_type from native char_traits)
            //REQUIRE(sz == sizeof(estd::span<uint32_t>) + sizeof(size_t));

            sb.pubseekoff(-1, estd::ios_base::cur);

            sb.sputc(4);

            // DEBT: Make this non-endian-specific
            REQUIRE(val[0] == 0x04020100);
        }
        SECTION("ostream usage (complex initialization)")
        {
            typedef estd::internal::impl::out_span_streambuf<uint32_t> sb_impl_type;
            typedef estd::internal::streambuf<sb_impl_type> sb_type;

            // Successfully cascades down 'val, 32' all the way down to out_span_streambuf
            estd::internal::basic_ostream<sb_type> out(val, 32);

            REQUIRE(out.rdbuf()->value().size() == 32);
        }
        SECTION("experimental")
        {
            SECTION("setbuf")
            {
                streambuf_impl_type sb((uint8_t*)&val[0], val_size);

                uint8_t* data = sb.pptr();

                // 32-bit 0 will not match 8-bit 0, 1, 2, 3 on 2nd byte
                // regardless of endianess (unless we have some flavor of
                // the double-swap little endian looking like 1, 0, 3, 2)
                REQUIRE(data[1] != test1[1]);

                sb.setbuf_experimental(test1, sizeof test1);

                data = sb.pptr();

                REQUIRE(data[0] == test1[0]);
                REQUIRE(data[1] == test1[1]);
                REQUIRE(data[2] == test1[2]);
                REQUIRE(data[3] == test1[3]);
            }
        }
    }
    SECTION("in_span_streambuf")
    {
        char buf[128];

        typedef estd::internal::impl::in_span_streambuf<const char> sb_impl_type;
        typedef estd::internal::streambuf<sb_impl_type> sb_type;

        sb_type sb(raw_str);

        SECTION("sgetn")
        {
            estd::streamsize read_back = sb.sgetn(buf, sizeof(buf));

            REQUIRE(sb.eback() == raw_str);
            // Remember, span streambuf is not a string streambuf, so we get the null
            // termination back also
            REQUIRE(read_back == raw_str_len + 1);
        }
        SECTION("sbumpc")
        {
            REQUIRE(sb.sbumpc() == raw_str[0]);
            REQUIRE(sb.sbumpc() == raw_str[1]);
            REQUIRE(sb.sbumpc() == raw_str[2]);
        }
        SECTION("pubseekoff")
        {
            int new_pos;

            new_pos = sb.pubseekoff(5, ios_base::cur);
            REQUIRE(new_pos == 5);
            new_pos = sb.pubseekoff(5, ios_base::cur);
            REQUIRE(new_pos == 10);
            REQUIRE(sb.sgetc() == raw_str[new_pos]);
        }
    }
    SECTION("pubsync - method finding")
    {
        struct pubsync_only_streambuf_impl : synthetic_streambuf_base
        {
            int sync() const { return 7; }
        };

        layer1::stringbuf<32> sb1;

        internal::streambuf<pubsync_only_streambuf_impl> sb2;

        REQUIRE(sb1.pubsync() == 0);
        REQUIRE(sb2.pubsync() == 7);
    }
    SECTION("ambiguous character availability")
    {
        struct synthetic_unavail_streambuf_impl : synthetic_streambuf_base
        {
            typedef synthetic_streambuf_base base_type;
        };

        internal::streambuf<synthetic_unavail_streambuf_impl> sb;

        REQUIRE(sb.in_avail() == 0);
    }
}