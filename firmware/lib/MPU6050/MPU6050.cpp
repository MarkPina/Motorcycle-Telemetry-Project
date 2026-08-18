#include "MPU6050.h"

bool MPU6050::begin(int sdaPin, int sclPin) {
    Wire.begin(sdaPin, sclPin);
    if (readRegister(kWhoAmI) != 0x68) {
        return false;
    }
    configure();
    return true;
}

void MPU6050::configure() {
    writeRegister(kPwrMgmt1,    0x00);
    writeRegister(kGyroConfig,  0x08);
    writeRegister(kAccelConfig, 0x08);
    writeRegister(kConfig,      0x05);
}

bool MPU6050::checkAwake() {
    if (++readCount_ % 100 != 0) {
        return true;
    }
    if (readRegister(kPwrMgmt1) != 0x00) {
        configure();
        return false;
    }
    return true;
}

bool MPU6050::read(Sample& out) {
    checkAwake();

    Wire.beginTransmission(kAddr);
    Wire.write(kAccelXoutH);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    if (Wire.requestFrom(kAddr, (uint8_t)14) != 14) {
        return false;
    }

    out.ax = read16() / kAccelScale;
    out.ay = read16() / kAccelScale;
    out.az = read16() / kAccelScale;
    read16();
    out.gx = read16() / kGyroScale;
    out.gy = read16() / kGyroScale;
    out.gz = read16() / kGyroScale;

    return true;
}

uint8_t MPU6050::readRegister(uint8_t reg) {
    Wire.beginTransmission(kAddr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(kAddr, (uint8_t)1);
    return Wire.read();
}

void MPU6050::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(kAddr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission(); 
}

int16_t MPU6050::read16() {
        uint8_t hi = Wire.read();
        uint8_t lo = Wire.read();
        return (int16_t)((hi << 8) | lo);
}