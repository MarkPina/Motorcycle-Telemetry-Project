#include <unity.h>
#include "RingBuffer.h"

void setUp(void) {}
void tearDown(void) {}

static Sample makeSample(uint32_t t) {
    Sample s;
    s.timestamp_ms = t;
    s.ax = 0.1f * t;  s.ay = 0.2f * t;  s.az = 1.0f;
    s.gx = 0.3f * t;  s.gy = 0.4f * t;  s.gz = 0.5f * t;
    return s;
}

void test_starts_empty(void) {
    RingBuffer rb;
    TEST_ASSERT_TRUE(rb.isEmpty());
    TEST_ASSERT_FALSE(rb.isFull());
    TEST_ASSERT_EQUAL_UINT32(0, rb.count());
}

void test_push_pop_preserves_value(void) {
    RingBuffer rb;
    rb.push(makeSample(42));

    Sample out;
    TEST_ASSERT_TRUE(rb.pop(out));
    TEST_ASSERT_EQUAL_UINT32(42, out.timestamp_ms);
    TEST_ASSERT_TRUE(rb.isEmpty());
}

void test_fifo_order(void) {
    RingBuffer rb;
    for (uint32_t i = 0; i < 10; i++) rb.push(makeSample(i));

    Sample out;
    for (uint32_t i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(rb.pop(out));
        TEST_ASSERT_EQUAL_UINT32(i, out.timestamp_ms);
    }
}

void test_pop_on_empty_fails(void) {
    RingBuffer rb;
    Sample out;
    TEST_ASSERT_FALSE(rb.pop(out));
}

void test_push_when_full_fails(void) {
    RingBuffer rb;
    for (uint32_t i = 0; i < 512; i++) {
        TEST_ASSERT_TRUE(rb.push(makeSample(i)));
    }
    TEST_ASSERT_TRUE(rb.isFull());
    TEST_ASSERT_FALSE(rb.push(makeSample(999)));
}

void test_wraps_around(void) {
    RingBuffer rb;
    Sample out;

    // fill, drain most, refill past the wrap point
    for (uint32_t i = 0; i < 512; i++) rb.push(makeSample(i));
    for (uint32_t i = 0; i < 500; i++) rb.pop(out);
    for (uint32_t i = 1000; i < 1400; i++) rb.push(makeSample(i));

    // oldest surviving sample should be 500
    TEST_ASSERT_TRUE(rb.pop(out));
    TEST_ASSERT_EQUAL_UINT32(500, out.timestamp_ms);
}

void test_high_water_mark(void) {
    RingBuffer rb;
    Sample out;

    for (uint32_t i = 0; i < 100; i++) rb.push(makeSample(i));
    for (uint32_t i = 0; i < 90; i++) rb.pop(out);

    TEST_ASSERT_EQUAL_UINT32(10, rb.count());
    TEST_ASSERT_EQUAL_UINT32(100, rb.highWaterMark());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_starts_empty);
    RUN_TEST(test_push_pop_preserves_value);
    RUN_TEST(test_fifo_order);
    RUN_TEST(test_pop_on_empty_fails);
    RUN_TEST(test_push_when_full_fails);
    RUN_TEST(test_wraps_around);
    RUN_TEST(test_high_water_mark);
    return UNITY_END();
}