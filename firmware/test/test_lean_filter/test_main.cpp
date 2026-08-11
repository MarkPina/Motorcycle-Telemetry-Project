#include <unity.h>
#include <cmath>
#include "LeanFilter.h"

static constexpr float kPi = 3.14159265358979f;
static constexpr float kGravity = 9.81f;
static constexpr float kDt = 0.01f;
static constexpr float kAlpha = 0.99f;
static constexpr float kLowSpeed = 3.0f;

void setUp(void) {}
void tearDown(void) {}

void test_converges_to_steady_turn(void) {
    LeanFilter filter(kAlpha, kLowSpeed);

    const float speed = 15.0f;
    const float targetPhi = 35.0f * kPi / 180.0f;
    const float yawRate = tanf(targetPhi) * kGravity / speed;

    for (int i = 0; i < 1000; i++) {
        filter.update(0.0f, yawRate, speed, 0.0f, 1.0f, kDt);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.01f, targetPhi, filter.getLeanAngle());
}

void test_rejects_constant_gyro_bias(void) {
    LeanFilter filter(kAlpha, kLowSpeed);

    const float speed = 15.0f;
    const float targetPhi = 35.0f * kPi / 180.0f;
    const float yawRate = tanf(targetPhi) * kGravity / speed;
    const float gyroBias = 0.02f;

    for (int i = 0; i < 1000; i++) {
        filter.update(gyroBias, yawRate, speed, 0.0f, 1.0f, kDt);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.05f, targetPhi, filter.getLeanAngle());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_converges_to_steady_turn);
    RUN_TEST(test_rejects_constant_gyro_bias);
    return UNITY_END();
}