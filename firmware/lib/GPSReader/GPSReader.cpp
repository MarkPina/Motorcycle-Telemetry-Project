#include "GPSReader.h"

GPSReader::GPSReader(HardwareSerial& port) : port_(port) {}

void GPSReader::begin(int rxPin, int txPin) {
    port_.begin(9600, SERIAL_8N1, rxPin, txPin);
    delay(100);

    setRate5Hz();
    delay(100);

    disableSentence(0x01);   // GLL
    disableSentence(0x02);   // GSA
    disableSentence(0x03);   // GSV
    disableSentence(0x05);   // VTG
}

void GPSReader::sendUbx(uint8_t cls, uint8_t id, const uint8_t* payload, uint8_t len) {
    uint8_t frame[32];

    frame[0] = 0xB5;         // sync char 1
    frame[1] = 0x62;         // sync char 2
    frame[2] = cls;
    frame[3] = id;
    frame[4] = len;          // length, little endian
    frame[5] = 0x00;

    for (uint8_t i = 0; i < len; i++) {
        frame[6 + i] = payload[i];
    }

    // Fletcher checksum over everything from the class byte to the end of payload
    uint8_t ckA = 0, ckB = 0;
    for (uint8_t i = 2; i < 6 + len; i++) {
        ckA += frame[i];
        ckB += ckA;
    }
    frame[6 + len]     = ckA;
    frame[6 + len + 1] = ckB;

    port_.write(frame, 6 + len + 2);
}

void GPSReader::setRate5Hz() {
    // UBX-CFG-RATE: measRate = 200 ms, navRate = 1, timeRef = GPS
    const uint8_t payload[] = { 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00 };
    sendUbx(0x06, 0x08, payload, sizeof(payload));
}

void GPSReader::disableSentence(uint8_t msgId) {
    // UBX-CFG-MSG: NMEA class 0xF0, given message, rate 0
    const uint8_t payload[] = { 0xF0, msgId, 0x00 };
    sendUbx(0x06, 0x01, payload, sizeof(payload));
    delay(50);
}

void GPSReader::poll() {
    while (port_.available()) {
        gps_.encode(port_.read());
    }
}

GPSFix GPSReader::snapshot() {
    GPSFix f;
    f.speed_mps   = gps_.speed.mps();
    f.heading_deg = gps_.course.deg();
    f.lat         = gps_.location.lat();
    f.lon         = gps_.location.lng();
    f.valid       = gps_.location.isValid()
    && gps_.speed.isValid()
    && gps_.location.age() < kMaxAgeMs
    && gps_.speed.age()    < kMaxAgeMs;
    return f;
}