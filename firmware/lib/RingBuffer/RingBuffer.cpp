#include "RingBuffer.h"

bool RingBuffer::push(const Sample& s) {
    if (count_ == kCapacity) {
        return false;
    }

    buffer_[head_] = s;
    head_ = (head_ + 1) % kCapacity;
    count_++;

    if (count_ > highWater_) {
        highWater_ = count_;
    }
    return true;
}

bool RingBuffer::pop(Sample& out) {
    if (count_ == 0) {
        return false;
    }

    out = buffer_[tail_];
    tail_ = (tail_ + 1) % kCapacity;
    count_--;
    return true;
}

size_t RingBuffer::count() const        { return count_; }
bool   RingBuffer::isEmpty() const      { return count_ == 0; }
bool   RingBuffer::isFull() const       { return count_ == kCapacity; }
size_t RingBuffer::highWaterMark() const { return highWater_; }