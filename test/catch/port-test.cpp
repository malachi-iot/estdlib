#include <catch2/catch.hpp>

#include <estd/string.h>

// DEBT: Move this out to a 'port.cpp' test
#include <estd/port/esp-idf/fake/esp_log.h>

static estd::layer1::string<256> fake_log_out;

static int fake_log_write(const char* fmt, va_list args)
{
    return vsnprintf(fake_log_out.data(), fake_log_out.max_size(), fmt, args);
}

TEST_CASE("port (cross plat specific) tests")
{
    esp_log_set_vprintf(fake_log_write);

    SECTION("fake ESP-IDF")
    {
        esp_log_write(ESP_LOG_INFO, "Tag1", "hello: %s", "2u");

        REQUIRE(fake_log_out == "hello: 2u");
    }
}