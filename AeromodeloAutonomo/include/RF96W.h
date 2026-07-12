#pragma once
#include <Arduino.h>
#include <RadioLib.h>

class LoRa {
public:

    LoRa(
        uint8_t cs,
        uint8_t dio0,
        uint8_t rst,

        float frequency = 915.0f,
        float bandwidth = 125.0f,
        uint8_t spreadingFactor = 10,
        uint8_t codingRate = 5,
        uint8_t syncWord = 0x12,
        int8_t power = 20,
        uint16_t preambleLength = 8
  
    );

    bool begin();

    bool send(const char* msg);

    bool available();
    bool receive();

    const char* getPacket() const;

    float getRSSI() const;
    float getSNR() const;

    void print();

private:

    static void setFlag();

    inline static volatile bool packetReceived = false;

    Module module;
    SX1276 radio;

    float frequency;
    float bandwidth;

    uint8_t spreadingFactor;
    uint8_t codingRate;
    uint8_t syncWord;

    int8_t power;
    uint16_t preambleLength;

    float rssi = 0.0f;
    float snr  = 0.0f;

    char packetBuffer[256];
};