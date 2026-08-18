#pragma once
#include <Arduino.h>
#include <TinyGPSPlus.h>

struct GPSFix {
    float  speed_mps;
    float  heading_deg;
    double lat;
    double lon;
    bool   valid;        // true only if fixed AND fresh
};

class GPSReader {
public:
    GPSReader(HardwareSerial& port);
    void begin(int rxPin, int txPin);
    void poll();                     // drain UART, non-blocking
    GPSFix snapshot();               // coherent set of current values

private:
    void sendUbx(uint8_t cls, uint8_t id, const uint8_t* payload, uint8_t len);
    void setRate5Hz();
    void disableSentence(uint8_t msgId);

    HardwareSerial& port_;
    TinyGPSPlus gps_;

    static constexpr uint32_t kMaxAgeMs = 1000;
};