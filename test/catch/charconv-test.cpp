#include <estd/charconv.h>
#include <estd/string.h>

#include "test-data.h"

#include <catch2/catch_all.hpp>

constexpr const char* test_str = "hello";
constexpr const char* test_str2 = "hi2u";

TEST_CASE("charconv")
{
    SECTION("from_chars")
    {
        SECTION("top level tests")
        {
            long output;

            SECTION("good")
            {
                estd::layer2::const_string val = "1234";
                estd::from_chars_result result = estd::from_chars(
                    val.data(), val.data() + val.size(), output);

                REQUIRE(result.ec == estd::errc{});
                REQUIRE(output == 1234);
            }
            SECTION("good 2")
            {
                estd::layer2::const_string val = "12a34";
                estd::from_chars_result result = estd::from_chars(
                    val.data(), val.data() + val.size(), output);

                REQUIRE(result.ec == estd::errc{});
                REQUIRE(output == 12);
            }
            SECTION("bad")
            {
                output = -7;
                estd::layer2::const_string val = test_str;
                estd::from_chars_result result = estd::from_chars(
                    val.data(), val.data() + val.size(), output);

                REQUIRE(result.ec == estd::errc::invalid_argument);
                REQUIRE(output == -7);
            }
            SECTION("overflow")
            {
                const char* val = "128";
                const char* val_end = val + strlen(val);

                int8_t output = -7;

                estd::from_chars_result result = estd::from_chars(
                    val, val_end, output);

                REQUIRE(result.ec == estd::errc::result_out_of_range);
                REQUIRE(output == -7);
                REQUIRE(result.ptr == val_end);
            }
        }
        SECTION("internal")
        {
            using namespace estd::internal;

            SECTION("base 2")
            {
                const char *src = "1010";

                short value = 0;
                from_chars_integer<2>(src, src + 4, value);

                REQUIRE(value == 10);
            }
            SECTION("base 10")
            {
                SECTION("positive")
                {
                    const char* src = "1234";
                    int value = 0;
                    from_chars_integer<10>(src, src + 4, value);

                    REQUIRE(value == 1234);
                }
                SECTION("negative")
                {
                    const char* src = "-1234";
                    int value = 0;
                    from_chars_integer<10>(src, src + 5, value);

                    REQUIRE(value == -1234);
                }
                SECTION("extra stuff")
                {
                    estd::layer2::const_string src = "1234 hello";

                    int value = 0;
                    estd::from_chars_result result =
                        from_chars_integer<10>(
                            src.data(),
                            src.data() + src.size(), value);

                    REQUIRE(result.ec == estd::errc{});
                    REQUIRE(value == 1234);
                    REQUIRE(estd::layer2::const_string(result.ptr) == " hello");
                }
            }
            SECTION("base 16")
            {
                const char *src = "FF";
                int value = 0;
                from_chars_integer<16>(src, src + 2, value);

                REQUIRE(value == 255);

                const char* src2 = "ab";

                from_chars_integer<16>(src2, src2 + 2, value);

                REQUIRE(value == 0xAB);
            }
            SECTION("sto mode")
            {
                estd::from_chars_result r;
                const char* src_pos = "+1234";
                const char* src_ws = "  -1234";
                const char* src_hex = "0xFF";
                int svalue = 0;
                unsigned uvalue = 0;

                SECTION("off (default)")
                {
                    r = from_chars_integer<10, false>(src_pos, src_pos + 5, svalue);
                    REQUIRE(r.ec == estd::errc::invalid_argument);
                    REQUIRE(svalue == 0);
                    r = from_chars_integer<10, false>(src_pos, src_pos + 5, uvalue);
                    r = from_chars_integer<10, false>(src_ws, src_ws + 7, svalue);
                    REQUIRE(r.ec == estd::errc::invalid_argument);
                    REQUIRE(svalue == 0);

                    // Bad hex still resolves the first '0' as a valid value
                    r = from_chars_integer<16, false>(src_hex, src_hex + 4, svalue);
                    REQUIRE(r.ec == estd::errc{});
                    REQUIRE(r.ptr == src_hex + 1);
                    REQUIRE(svalue == 0);
                }
                SECTION("on")
                {
                    r = from_chars_integer<10, true>(src_pos, src_pos + 5, svalue);
                    REQUIRE(svalue == 1234);
                    r = from_chars_integer<10, true>(src_pos, src_pos + 5, uvalue);
                    REQUIRE(r.ec == estd::errc{});
                    REQUIRE(uvalue == 1234);
                    r = from_chars_integer<10, true>(src_ws, src_ws + 7, svalue);
                    REQUIRE(svalue == -1234);
                    r = from_chars_integer<16, true>(src_hex, src_hex + 4, svalue);
                    REQUIRE(svalue == 255);

                    r = estd::from_chars<int, true>(src_hex, src_hex + 4, svalue, 0);
                    REQUIRE(r.ec == estd::errc{});
                    REQUIRE(svalue == 255);
                }
            }
        }
        SECTION("various types")
        {
            SECTION("long")
            {
                estd::layer2::const_string s = "1234";
                long value{};

                estd::from_chars_result result =
                    estd::from_chars(s.data(), s.data() + s.size(), value);

                REQUIRE(result.ec == estd::errc{});
                REQUIRE(value == 1234);
            }
            SECTION("unsigned")
            {
                estd::layer2::const_string s = "1234";
                unsigned value{};

                estd::from_chars_result result =
                    estd::from_chars(s.data(), s.data() + s.size(), value);

                REQUIRE(result.ec == estd::errc{});
                REQUIRE(value == 1234);
            }
            SECTION("lazy")
            {
                const char s[128] = "1234";

                unsigned value{};

                estd::from_chars_result result =
                    estd::from_chars(s, &s[0] + sizeof(s), value);

                REQUIRE(result.ec == estd::errc{});
                REQUIRE(value == 1234);
            }
            SECTION("float")
            {
                const char s[] = "123.456";
                float value{};

                estd::from_chars_result result =
                    estd::from_chars(s, &s[0] + sizeof(s), value);

                REQUIRE(result.ec == estd::errc{});
                REQUIRE_THAT(value, Catch::Matchers::WithinAbs(123.456, 0.00001));
                //REQUIRE(result.ptr == &s[0] + sizeof(s));
            }
        }
        SECTION("sto")
        {
            SECTION("on")
            {

            }
            SECTION("off (default)")
            {

            }
        }
    }
    SECTION("to_chars")
    {
        char buffer[128];

        int inputs[] { 771, 5, 0, -100 };
        const char* outputs[] { "771", "5", "0", "-100" };
        constexpr int sz = sizeof(inputs) / sizeof(int);

        SECTION("standard")
        {
            for(int i = 0; i < sz; ++i)
            {
                estd::to_chars_result result = estd::to_chars(&buffer[0], &buffer[127], inputs[i]);

                REQUIRE(result.ec == estd::errc{});

                *result.ptr = 0;

                REQUIRE(std::string(buffer) == outputs[i]);
            }
        }
        SECTION("opt")
        {
            for(int i = 0; i < sz; ++i)
            {
                // NOTE: Remember, to_chars is not a null terminating create, so we must
                // designate the end a little earlier so there's room for the null terminator
                estd::to_chars_result result = estd::to_chars_opt(buffer, buffer + sizeof(buffer) - 1, inputs[i]);

                REQUIRE(result.ec == estd::errc{});

                buffer[127] = 0;

                REQUIRE(std::string(result.ptr) == outputs[i]);
            }
        }
        SECTION("reverse method")
        {
            estd::to_chars_result result = estd::to_chars_exp(&buffer[0], &buffer[127], 255);

            REQUIRE(result.ec == estd::errc{});

            *result.ptr = 0;

            REQUIRE(std::string(buffer) == "255");
        }
        SECTION("into layer1 string")
        {
            estd::layer1::string<32> s;

            estd::to_chars_result result = estd::to_chars(s.data(), s.data() + 32, 771);

            auto i = s[0];

            REQUIRE(i == '7');
            REQUIRE(s[1] == '7');
            REQUIRE(s[2] == '1');

            // DEBT: errc needs work, needs a default constructor and ability to == and friends
            //REQUIRE(result.ec == estd::errc(0));
            REQUIRE(result.ec == estd::errc{});
        }
        SECTION("base 16")
        {
            estd::to_chars_result result = estd::detail::to_chars<16>(buffer, buffer + sizeof(buffer) - 1, 10);

            REQUIRE(*result.ptr == 'a');
        }
    }
    SECTION("char_traits")
    {
        std::char_traits<const char>::length("hi2u");
    }
}
