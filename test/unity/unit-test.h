#pragma once

// NOTE: math.h is necessary before including unity.h
#include <math.h>
#include "unity.h"

#include <estd/internal/platform.h>

// TODO: Make ESP_IDF_TESTING something specified/deduced from
// native esp-idf testing rather than guessed based on it being arduino
// (doing this temporarily to bring up pio unity testing)
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#define ESP_IDF_TESTING
#endif

void test_align();
void test_array();
void test_chrono();
void test_cstddef();
void test_optional();
void test_queue();
void test_ratio();
void test_streambuf();;
void test_string();
void test_thread();
void test_map();

struct TestStruct
{
    int value;

    TestStruct(int value) : value(value) {}

    // DEBT: Necessary to participate in layer1::queue due to incomplete implementation of
    // aligned_storage
    TestStruct() {}
};
