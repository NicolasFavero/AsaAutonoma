#pragma once
#include <Arduino.h>
struct AttitudeData{
    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;
 
    float accX = 0.0f;
    float accY = 0.0f;
    float accZ = 0.0f;

    float gyroX = 0.0f;
    float gyroY = 0.0f;
    float gyroZ = 0.0f;

    float magX = 0.0f;
    float magY = 0.0f;
    float magZ = 0.0f;
};

struct NavigationData{
    double latitude = 0.0f;
    double longitude = 0.0f;

    float gpsAltitude = 0.0f;
    float course = 0.0f;

    uint8_t satellites = 0;

    float baroAltitude = 0.0f;
    float temperature = 0.0f;
    float battery = 0.0f;
};

struct ServoPositions{
    float elevator = 90.0f;
    float leftAlieron = 90.0f;
    float rightAlieron = 90.0f;
    //float rudder = 90.0f;
};
/*==========================================================
                    IMU Offsets
==========================================================*/

struct ImuOffsets
{
    float pitch = 0.0f;

    float roll = 0.0f;

    float yaw = 0.0f;
};

struct LoraConfig
{
    float frequency = 915.0f;

    float bandwidth = 125.0f;

    uint8_t spreadingFactor = 10;

    uint8_t codingRate = 5;

    uint8_t power = 20;

    uint16_t preambleLength = 8;

    uint8_t syncWord = 0x12;
};

enum class FlightMode : uint8_t
{
    CONFIG,

    COUNTDOWN,

    FLIGHT,

    LANDED
};


enum class SystemEvent : uint8_t
{
    NONE = 0,

    OFFSET_CHANGED,

    SYSTEM_CHANGED,

    LORA_CHANGED,

    START_FLIGHT,

    RESTART
};


struct SystemConfig
{
    float batteryLimit = 7.5f;

    uint16_t telemetryPeriodMs = 1E3;

    uint16_t lowBatteryTelemetryPeriodMs = 1E4;

    uint16_t estimatedFlightTimeMin = 30;

    bool wifiEnabled = true;

    bool telemetryWebEnabled = false;

    FlightMode flightMode = FlightMode::CONFIG;

    LoraConfig lora;
};


struct SystemStatus
{
    bool imuOk = false;

    bool gpsOk = false;

    bool bmpOk = false;

    bool adsOk = false;

    bool loraOk = false;

    bool sdOk = false;

    bool wifiRunning = false;

    bool wifiClientConnected = false;

    uint8_t wifiClients = 0;
};

