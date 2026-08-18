#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "RingBuffer.h"   // for Sample

class MPU6050 {
public:
    bool begin(int sdaPin, int sclPin);
    bool read(Sample& out);          // fills accel (g) and gyro (deg/s)

private:
    uint8_t  readRegister(uint8_t reg);
    void     writeRegister(uint8_t reg, uint8_t value);
    int16_t  read16();
    void     configure();            // the four writeRegister calls
    bool     checkAwake();           // the periodic sleep-recovery check

    uint32_t readCount_ = 0;

    static constexpr uint8_t kAddr        = 0x68;
    static constexpr uint8_t kWhoAmI      = 0x75;
    static constexpr uint8_t kPwrMgmt1    = 0x6B;
    static constexpr uint8_t kGyroConfig  = 0x1B;
    static constexpr uint8_t kAccelConfig = 0x1C;
    static constexpr uint8_t kConfig      = 0x1A;
    static constexpr uint8_t kAccelXoutH  = 0x3B;

    static constexpr float kAccelScale = 8192.0f;   // ±4 g
    static constexpr float kGyroScale  = 65.5f;     // ±500 dps
};