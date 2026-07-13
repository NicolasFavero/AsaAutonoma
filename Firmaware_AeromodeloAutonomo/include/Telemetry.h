#pragma once
#include <Arduino.h>

#include "Imu.h"
#include "GPS.h"
#include "BMP280.h"
#include "ADS1X15.h"

enum FlightState
{
    WAITING,
    FLYING,
    LANDED
};


class Telemetry
{

public:

    Telemetry(IMU& imu, GPS& gps, BMP280& bmp, ADS& ads);


    void update();


    const char* getJson() const;
    const char* getCsv() const;
    const char* getCsvHeader() const;

    const char* getLoraPacket(bool reduced);


    void setState(FlightState newState);


private:

    IMU& imu;
    GPS& gps;
    BMP280& bmp;
    ADS& ads;


    FlightState state = WAITING;


    char json[256];
    char csv[600];
    char loraPacket[256];


    void buildJson();
    void buildCsv();


    const char* stateToString() const;

};