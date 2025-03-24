#include <catch2/catch_all.hpp>

#include <estd/string.h>

// DEBT: Move this out to a 'port.cpp' test
#include <estd/port/esp-idf/fake/esp_log.h>

static estd::layer1::string<256> fake_log_out;
static bool newline_encountered;

static int fake_log_write(const char* fmt, va_list args)
{
    if(newline_encountered)
    {
        fake_log_out.clear();
        newline_encountered = false;
    }

    if(fmt[0] == '\n')  newline_encountered = true;

    unsigned sz = fake_log_out.size();
    unsigned max = fake_log_out.max_size() - sz;
    return vsnprintf(fake_log_out.data() + sz, max, fmt, args);
}

TEST_CASE("port (cross plat specific) tests")
{
    esp_log_set_vprintf(fake_log_write);

    SECTION("fake ESP-IDF")
    {
        esp_log_write(ESP_LOG_INFO, "Tag1", "hello: %s", "2u");

        REQUIRE(fake_log_out == "I Tag1: hello: 2u\n");

        //esp_log_write(ESP_LOG_INFO, "Tag2", "hello: %s", "also");

        ESP_LOGE("Tag2", "uh oh! %d", 5);

        REQUIRE(fake_log_out == "E Tag2: uh oh! 5\n");

        ESP_LOGW("Tag2", "Goodbye");
    }
}
