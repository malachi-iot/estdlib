#include <estd/charconv.h>
#include <estd/string.h>

#include "test-data.h"

#include <catch.hpp>

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

                REQUIRE(result.ec == 0);
                REQUIRE(output == 1234);
            }
            SECTION("good 2")
            {
                estd::layer2::const_string val = "12a34";
                estd::from_chars_result result = estd::from_chars(
                    val.data(), val.data() + val.size(), output);

                REQUIRE(result.ec == 0);
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

                    REQUIRE(result.ec == 0);
                    REQUIRE(value == 1234);
                    REQUIRE(estd::layer2::const_string(result.ptr) == " hello");
                }
            }
            SECTION("base 16")
            {
                const char *src = "FF";
                int value = 0;
                from_chars_integer<16>(src, src + 4, value);

                REQUIRE(value == 255);
            }
        }
        SECTION("various types")
        {
            SECTION("long")
            {
                estd::layer2::const_string s = "1234";
                long value;

                estd::from_chars_result result =
                    estd::from_chars(s.data(), s.data() + s.size(), value);

                REQUIRE(result.ec == 0);
                REQUIRE(value == 1234);
            }
            SECTION("unsigned")
            {
                estd::layer2::const_string s = "1234";
                unsigned value;

                estd::from_chars_result result =
                    estd::from_chars(s.data(), s.data() + s.size(), value);

                REQUIRE(result.ec == 0);
                REQUIRE(value == 1234);
            }
            SECTION("lazy")
            {
                const char s[128] = "1234";

                unsigned value;

                estd::from_chars_result result =
                    estd::from_chars(s, &s[0] + sizeof(s), value);

                REQUIRE(result.ec == 0);
                REQUIRE(value == 1234);
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

                REQUIRE(result.ec == 0);

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
                estd::to_chars_result result = estd::to_chars_opt(&buffer[0], &buffer[126], inputs[i]);

                REQUIRE(result.ec == 0);

                buffer[127] = 0;

                REQUIRE(std::string(result.ptr) == outputs[i]);
            }
        }
        SECTION("reverse method")
        {
            estd::to_chars_result result = estd::to_chars_exp(&buffer[0], &buffer[127], 255);

            REQUIRE(result.ec == 0);

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
            REQUIRE(result.ec == 0);
        }
        SECTION("base 16")
        {
            estd::to_chars_result result = estd::to_chars_opt(buffer, buffer + sizeof(buffer) - 1, 10, 16);

            REQUIRE(*result.ptr == 'a');
        }
    }
}