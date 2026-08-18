#include <Arduino.h>
#include "MPU6050.h"
#include "LeanFilter.h"
#include "RingBuffer.h"

// --- pins ---
const int I2C_SDA = 21;
const int I2C_SCL = 22;

// --- tuning ---
const float kAlpha        = 0.99f;
const float kLowSpeed     = 3.0f;
const float kDegToRad     = 0.0174533f;
const float kRadToDeg     = 57.29578f;
const float kFakeSpeedMps = 15.0f;   // TEMPORARY: real GPS arrives in step 4

// --- shared objects ---
MPU6050 imu;
LeanFilter filter(kAlpha, kLowSpeed);
QueueHandle_t imuQueue;

void imuTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        Sample s;
        if (imu.read(s)) {
            s.timestamp_ms = millis();
            xQueueSend(imuQueue, &s, 0);
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));   // 100 Hz
    }
}

void fusionTask(void* pvParameters) {
    Sample s;
    uint32_t lastMicros = micros();
    uint32_t lastPrint  = 0;

    for (;;) {
        if (xQueueReceive(imuQueue, &s, portMAX_DELAY) == pdTRUE) {
            uint32_t now = micros();
            float dt = (now - lastMicros) / 1000000.0f;
            lastMicros = now;

            float rollRate = s.gx * kDegToRad;
            float yawRate = -s.gz * kDegToRad;

            filter.update(rollRate, yawRate, kFakeSpeedMps, s.ay, s.az, dt);
            s.lean_deg = filter.getLeanAngle() * kRadToDeg;

            if (millis() - lastPrint > 100) {
                lastPrint = millis();
                Serial.print("a: ");
                Serial.print(s.ax, 2); Serial.print(" ");
                Serial.print(s.ay, 2); Serial.print(" ");
                Serial.print(s.az, 2);
                Serial.print("   g: ");
                Serial.print(s.gx, 1); Serial.print(" ");
                Serial.print(s.gy, 1); Serial.print(" ");
                Serial.print(s.gz, 1);
                Serial.print("   lean: ");
                Serial.println(s.lean_deg, 2);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!imu.begin(I2C_SDA, I2C_SCL)) {
        Serial.println("MPU init FAILED");
        for (;;) delay(1000);
    }
    Serial.println("MPU init OK");

    imuQueue = xQueueCreate(10, sizeof(Sample));
    if (imuQueue == nullptr) {
        Serial.println("queue create FAILED");
        for (;;) delay(1000);
    }

    xTaskCreatePinnedToCore(imuTask,    "imu",    4096, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(fusionTask, "fusion", 4096, nullptr, 4, nullptr, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}