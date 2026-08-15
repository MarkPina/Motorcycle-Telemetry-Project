#include <Arduino.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;

void setGpsRate5Hz() {
    // UBX-CFG-RATE: measRate=200ms, navRate=1, timeRef=1 (GPS)
    const uint8_t cmd[] = {
        0xB5, 0x62,             // sync chars
        0x06, 0x08,             // class CFG, id RATE
        0x06, 0x00,             // payload length = 6
        0xC8, 0x00,             // measRate = 200 ms (little endian)
        0x01, 0x00,             // navRate = 1
        0x01, 0x00,             // timeRef = GPS
        0xDE, 0x6A              // checksum A, B
    };
    Serial2.write(cmd, sizeof(cmd));
}

void disableSentence(uint8_t msgId) {
    uint8_t cmd[] = {
        0xB5, 0x62, 0x06, 0x01, 0x03, 0x00,
        0xF0, msgId, 0x00, 0x00, 0x00
    };
    // compute checksum
    uint8_t ckA = 0, ckB = 0;
    for (int i = 2; i < 9; i++) { ckA += cmd[i]; ckB += ckA; }
    cmd[9] = ckA;
    cmd[10] = ckB;
    Serial2.write(cmd, sizeof(cmd));
    delay(50);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    setGpsRate5Hz();
    delay(100);

     disableSentence(0x01); // GLL
    disableSentence(0x02); // GSA
    disableSentence(0x03); // GSV
    disableSentence(0x05); // VTG

    Serial.println("GPS parse test");
}

void loop() {
    static uint32_t sentenceCount = 0;

    while (Serial2.available()) {
        if (gps.encode(Serial2.read())) {
            sentenceCount++;
        }
    }

    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 1000) {
        lastPrint = millis();

        Serial.print("Sentences/sec: ");
        Serial.print(sentenceCount);
        sentenceCount = 0;

        Serial.print("  Fix: ");
        Serial.print(gps.location.isValid() ? "YES" : "NO");
        Serial.print("  Sats: ");
        Serial.print(gps.satellites.value());
        Serial.print("  HDOP: ");
        Serial.print(gps.hdop.hdop(), 2);
        Serial.print("  Lat: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print("  Lon: ");
        Serial.print(gps.location.lng(), 6);
        Serial.print("  Speed(m/s): ");
        Serial.print(gps.speed.mps(), 2);
        Serial.print("  Course: ");
        Serial.println(gps.course.deg(), 1);
        Serial.print("  Time: ");
        Serial.println(lastPrint);
    }
}