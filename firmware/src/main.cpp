#include <Arduino.h>
#include <Wire.h>

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;
const uint8_t ACCEL_CONFIG = 0x1C;
const uint8_t ACCEL_XOUT_H = 0x3B;
const uint8_t CONFIG = 0x1A;

uint8_t readRegister(uint8_t reg) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);   // repeated start, don't release the bus
    Wire.requestFrom(MPU_ADDR, (uint8_t)1);
    return Wire.read();
}

void writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();   // true — release the bus, we're done
}

int16_t read16() {
        uint8_t hi = Wire.read();
        uint8_t lo = Wire.read();
        return (int16_t)((hi << 8) | lo);
    }

void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);

    Serial.print("WHO_AM_I = 0x");
    Serial.println(readRegister(WHO_AM_I), HEX);

    writeRegister(PWR_MGMT_1,0x00);
    writeRegister(GYRO_CONFIG,0x08);
    writeRegister(ACCEL_CONFIG,0x08);
    writeRegister(CONFIG,0x05);

    Serial.print("PWR_MGMT_1 = 0x");
    Serial.println(readRegister(PWR_MGMT_1), HEX);

}

void loop() {

    if (readRegister(PWR_MGMT_1) != 0x00) {
        Serial.println("MPU asleep — reconfiguring");
        writeRegister(PWR_MGMT_1, 0x00);
        writeRegister(GYRO_CONFIG, 0x08);
        writeRegister(ACCEL_CONFIG, 0x08);
        writeRegister(CONFIG, 0x05);
    }

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(ACCEL_XOUT_H);
    if (Wire.endTransmission(false) != 0) {
        Serial.println("I2C write failed");
        return;
    }
    if (Wire.requestFrom(MPU_ADDR, (uint8_t)14) != 14) {
        Serial.println("I2C read short");
        return;
    }

    int16_t ax = read16();
    int16_t ay = read16();
    int16_t az = read16();
    int16_t temp = read16();
    int16_t gx = read16();
    int16_t gy = read16();
    int16_t gz = read16();

    Serial.print(ax / 8192.0f); Serial.print("\t");
    Serial.print(ay / 8192.0f); Serial.print("\t");
    Serial.print(az / 8192.0f); Serial.print("\t");
    Serial.print(gx / 65.5f);   Serial.print("\t");
    Serial.print(gy / 65.5f);   Serial.print("\t");
    Serial.println(gz / 65.5f);

    delay(100);
}