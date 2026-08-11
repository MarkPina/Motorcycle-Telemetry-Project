#pragma once

class LeanFilter {
    public:
        LeanFilter(float alpha, float lowSpeedThreshold);

        void update (float rollRate, float yawRate, float speed,
                    float accelY, float accelZ, float dt);

        float getLeanAngle() const; // radians

    private:
        float phi_;
        float alpha_;
        float lowSpeedThreshold_;
};