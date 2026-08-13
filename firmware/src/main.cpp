#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

const int SD_CS = 5;

void setup() {
    Serial.begin(115200);
    delay(1000);

    SD.remove("/test.csv");           // wipe before writing

    if (!SD.begin(SD_CS, SPI, 1000000)) {
        Serial.println("SD init FAILED");
        return;
    }
    Serial.println("SD init OK");

    File f = SD.open("/test.csv", FILE_WRITE);
    if (!f) {
        Serial.println("File open FAILED");
        return;
    }

    size_t written = f.println("timestamp_ms,ax,ay,az,gx,gy,gz");

    if (written == 0) {
        Serial.println("Header write FAILED");
    }

    for (int i = 0; i < 100; i++) {
        f.print(i * 10);
        f.print(",");
        f.print(0.01f * i, 4);
        f.print(",");
        f.print(0.02f * i, 4);
        f.print(",");
        f.print(1.0f, 4);
        f.print(",");
        f.print(0.1f * i, 4);
        f.print(",");
        f.print(0.2f * i, 4);
        f.print(",");
        f.println(0.3f * i, 4);
    }

    Serial.print("File size: ");
    Serial.println(f.size());

    f.close();
    Serial.println("Wrote 100 rows to /test.csv");
}

void loop() {}