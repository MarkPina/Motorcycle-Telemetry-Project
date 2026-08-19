#include <Arduino.h>
#include "MPU6050.h"
#include "LeanFilter.h"
#include "RingBuffer.h"
#include "GPSReader.h"
#include <SPI.h>
#include <SD.h>
#include "SafeRingBuffer.h"

// --- pins ---
const int I2C_SDA = 21;
const int I2C_SCL = 22;
const int GPS_RX  = 16;
const int GPS_TX  = 17;
const int SD_CS = 5;

// --- tuning ---
const float kAlpha        = 0.99f;
const float kLowSpeed     = 3.0f;
const float kDegToRad     = 0.0174533f;
const float kRadToDeg     = 57.29578f;

// --- shared objects ---
MPU6050 imu;
LeanFilter filter(kAlpha, kLowSpeed);
QueueHandle_t imuQueue;
GPSReader gps(Serial2);
QueueHandle_t gpsMailbox;
SafeRingBuffer buffer;
File logFile;
char logLine[160];
char logBatch[16384];
uint32_t sdErrors = 0;

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

            GPSFix fix = {};
            xQueuePeek(gpsMailbox, &fix, 0);

            float rollRate = s.gx * kDegToRad;
            float yawRate = -s.gz * kDegToRad;
            float speed = fix.valid ? fix.speed_mps : 0.0f;

            filter.update(rollRate, yawRate, speed, s.ay, s.az, dt);
            s.lean_deg  = filter.getLeanAngle() * kRadToDeg;
            s.speed_mps = fix.speed_mps;
            s.heading_deg = fix.heading_deg;
            s.lat = fix.lat;
            s.lon = fix.lon;
            s.fix = fix.valid;

            buffer.push(s);

            if (millis() - lastPrint > 100) {
                lastPrint = millis();
                Serial.print("lean: ");   Serial.print(s.lean_deg, 2);
                Serial.print("   fix: ");  Serial.print(s.fix ? "Y" : "N");
                Serial.print("   spd: ");  Serial.print(s.speed_mps, 1);
                Serial.print("   sats-lat: "); Serial.print(s.lat, 5);
                Serial.print("   gz: ");   Serial.print(s.gz, 1);
                Serial.print("   buf: ");   Serial.print(buffer.count());
                Serial.print("   hwm: ");   Serial.print(buffer.highWaterMark());
                Serial.print("   drop: ");  Serial.print(buffer.droppedCount());
                Serial.print("   sderr: "); Serial.println(sdErrors);
            }
        }
    }
}

void gpsTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        gps.poll();
        GPSFix fix = gps.snapshot();
        xQueueOverwrite(gpsMailbox, &fix);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(50));   // 20 Hz
    }
}

void flushBatch(size_t len) {
    size_t written = logFile.write((const uint8_t*)logBatch, len);
    if (written != len) {
        sdErrors++;
    }
    logFile.flush();
}

void loggerTask(void* pvParameters) {
    size_t used = 0;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        Sample s;
        while (buffer.pop(s)) {
            int n = snprintf(logLine, sizeof(logLine),
                "%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.2f,%.1f,%.6f,%.6f,%d,%.2f\n",
                s.timestamp_ms,
                s.ax, s.ay, s.az,
                s.gx, s.gy, s.gz,
                s.speed_mps, s.heading_deg,
                s.lat, s.lon,
                s.fix ? 1 : 0,
                s.lean_deg);

            if (used + n >= (int)sizeof(logBatch)) {
                flushBatch(used);
                used = 0;
            }
            memcpy(logBatch + used, logLine, n);
            used += n;
        }

        if (used > 0) {
            flushBatch(used);
            used = 0;
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(500));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    gps.begin(GPS_RX, GPS_TX);

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

    gpsMailbox = xQueueCreate(1, sizeof(GPSFix));
    if (gpsMailbox == nullptr) {
        Serial.println("GPS mailbox create FAILED");
        for (;;) delay(1000);
    }

    if (!buffer.begin()) {
    Serial.println("buffer mutex FAILED");
    for (;;) delay(1000);
    }

    if (!SD.begin(SD_CS, SPI, 1000000)) {
        Serial.println("SD init FAILED");
        for (;;) delay(1000);
    }
    Serial.println("SD init OK");

    logFile = SD.open("/ride.csv", FILE_WRITE);
    if (!logFile) {
        Serial.println("file open FAILED");
        for (;;) delay(1000);
    }
    logFile.println("timestamp_ms,ax,ay,az,gx,gy,gz,speed_mps,heading_deg,lat,lon,fix,lean_deg");

    xTaskCreatePinnedToCore(imuTask,    "imu",    4096, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(fusionTask, "fusion", 4096, nullptr, 4, nullptr, 1);
    xTaskCreatePinnedToCore(gpsTask,    "gps",    4096, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(loggerTask, "logger", 4096, nullptr, 2, nullptr, 0);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}