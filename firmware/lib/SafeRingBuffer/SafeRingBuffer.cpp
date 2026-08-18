#include "SafeRingBuffer.h"

bool SafeRingBuffer::push(const Sample& s) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    bool ok = rb_.push(s);
    if (!ok) dropped_++;
    xSemaphoreGive(mutex_);
    return ok;
}

bool SafeRingBuffer::pop(Sample& out) {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    bool ok = rb_.pop(out);
    xSemaphoreGive(mutex_);
    return ok;
}

bool SafeRingBuffer::begin() {
    mutex_ = xSemaphoreCreateMutex();
    return mutex_ != nullptr;
}

size_t SafeRingBuffer::count() {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    size_t n = rb_.count();
    xSemaphoreGive(mutex_);
    return n;
}

size_t SafeRingBuffer::highWaterMark() {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    size_t n = rb_.highWaterMark();
    xSemaphoreGive(mutex_);
    return n;
}

uint32_t SafeRingBuffer::droppedCount() {
    xSemaphoreTake(mutex_, portMAX_DELAY);
    uint32_t n = dropped_;
    xSemaphoreGive(mutex_);
    return n;
}