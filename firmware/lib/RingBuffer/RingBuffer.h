#pragma once
#include <cstdint>
#include <cstddef>

struct Sample {
    uint32_t timestamp_ms;
    float ax, ay, az;
    float gx, gy, gz;
    float speed_mps;
    float heading_deg;
    double lat, lon;
    bool  fix;
    float lean_deg;
};

class RingBuffer {
public:
    bool push(const Sample& s);
    bool pop(Sample& out);

    size_t count() const;
    bool isEmpty() const;
    bool isFull() const;
    size_t highWaterMark() const;

private:
    static constexpr size_t kCapacity = 512;
    Sample buffer_[kCapacity];
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
    size_t highWater_ = 0;
};