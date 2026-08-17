#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "RingBuffer.h"

const int SD_CS = 5;
RingBuffer rb;
File logFile;

// timing stats
uint32_t writeCount = 0;
uint32_t writeMaxUs = 0;
uint32_t writeTotalUs = 0;

char lineBuf[128];
String batch;

void flushBatch() {
    if (batch.length() == 0) return;

    uint32_t t0 = micros();
    logFile.print(batch);
    logFile.flush();
    uint32_t dt = micros() - t0;

    writeCount++;
    writeTotalUs += dt;
    if (dt > writeMaxUs) writeMaxUs = dt;

    batch = "";
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!SD.begin(SD_CS, SPI, 1000000)) {
        Serial.println("SD init FAILED");
        return;
    }
    Serial.println("SD init OK");

    SD.remove("/batch.csv");
    logFile = SD.open("/batch.csv", FILE_WRITE);
    if (!logFile) {
        Serial.println("open FAILED");
        return;
    }
    logFile.println("timestamp_ms,ax,ay,az,gx,gy,gz");
    batch.reserve(4096);

    // produce 2000 synthetic samples at full tilt
    for (uint32_t i = 0; i < 2000; i++) {
        Sample s;
        s.timestamp_ms = i * 10;
        s.ax = 0.01f * i;  s.ay = 0.02f * i;  s.az = 1.0f;
        s.gx = 0.1f * i;   s.gy = 0.2f * i;   s.gz = 0.3f * i;

        if (!rb.push(s)) {
        Serial.println("!! ring buffer full");
        }

        // drain in batches of 50
        if (rb.count() >= 50) {
        Sample out;
        while (rb.pop(out)) {
            snprintf(lineBuf, sizeof(lineBuf),
                    "%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                    out.timestamp_ms, out.ax, out.ay, out.az,
                    out.gx, out.gy, out.gz);
            batch += lineBuf;
        }
        flushBatch();
        }
    }

    // drain whatever's left
    Sample out;
    while (rb.pop(out)) {
        snprintf(lineBuf, sizeof(lineBuf),
                "%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                out.timestamp_ms, out.ax, out.ay, out.az,
                out.gx, out.gy, out.gz);
        batch += lineBuf;
    }
    flushBatch();

    size_t finalSize = logFile.size();
    logFile.close();

    Serial.println("--- results ---");
    Serial.print("file size: ");      Serial.println(finalSize);
    Serial.print("batch writes: ");   Serial.println(writeCount);
    Serial.print("max write us: ");   Serial.println(writeMaxUs);
    Serial.print("mean write us: ");  Serial.println(writeTotalUs / writeCount);
    Serial.print("buffer high water: "); Serial.println(rb.highWaterMark());
}

void loop() {}