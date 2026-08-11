#include "LeanFilter.h"
#include <cmath>

static constexpr float kGravity = 9.81f;

LeanFilter::LeanFilter(float alpha, float lowSpeedThreshold)
    : phi_(0.0f), alpha_(alpha), lowSpeedThreshold_(lowSpeedThreshold){}

float LeanFilter::getLeanAngle() const {
    return phi_;
}

void LeanFilter::update(float rollRate, float yawRate, float speed,
                        float accelY, float accelZ, float dt){
    float phiRef;
    if (speed >= lowSpeedThreshold_){
        phiRef = atan2f(speed * yawRate, kGravity);
    }
    else {
        phiRef = atan2f(accelY, accelZ);
    }

    float phiGyro = phi_ + rollRate * dt;

    phi_ = alpha_ * phiGyro + (1.0f - alpha_) * phiRef;
}