#pragma once
#include <Arduino.h>
struct AttitudeData
{
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