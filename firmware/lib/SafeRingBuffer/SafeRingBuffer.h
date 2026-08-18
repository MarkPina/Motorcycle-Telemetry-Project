#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "RingBuffer.h"

class SafeRingBuffer {
public:
    bool begin();                    // creates the mutex; false if allocation fails

    bool push(const Sample& s);
    bool pop(Sample& out);

    size_t count();
    size_t highWaterMark();
    uint32_t droppedCount();

private:
    RingBuffer rb_;
    SemaphoreHandle_t mutex_ = nullptr;
    uint32_t dropped_ = 0;
};