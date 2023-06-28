#pragma once

// NOTE: math.h is necessary before including unity.h
#include <math.h>
#include <unity.h>

#include <estd/internal/platform.h>

// TODO: Make ESP_IDF_TESTING something specified/deduced from
// native esp-idf testing rather than guessed based on it being arduino
// (doing this temporarily to bring up pio unity testing)
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#define ESP_IDF_TESTING
#endif

void test_align();
void test_array();
void test_cpp();
void test_chrono();
void test_cstddef();
void test_expected();
void test_functional();
void test_limits();
void test_locale();
void test_optional();
void test_ostream();
void test_queue();
void test_ratio();
void test_span();
void test_streambuf();
void test_string();
void test_thread();
void test_tuple();
void test_type_traits();
void test_map();
void test_variant();

struct TestStruct
{
    int value;

    TestStruct(int value) : value(value) {}

    // DEBT: Necessary to participate in layer1::queue due to incomplete implementation of
    // aligned_storage
    TestStruct() {}
};
