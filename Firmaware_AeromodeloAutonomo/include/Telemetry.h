#pragma once
#include <Arduino.h>

#include "GPS.h"
#include "BMP280.h"
#include "ADS1X15.h"
#include "DataTypes.h"

enum FlightState
{
    WAITING,
    FLYING,
    LANDED
};

class Telemetry
{
public:

    Telemetry(GPS& gps, BMP280& bmp, ADS& ads);

    void setAttitudeData(const AttitudeData& data);

    void update(bool imuValid);

    const char* getJson() const;
    const char* getCsv() const;
    const char* getCsvHeader() const;

    const char* getLoraPacket(bool reduced);

    void setState(FlightState newState);

private:

    GPS& gps;
    BMP280& bmp;
    ADS& ads;

    AttitudeData attitude;

    FlightState state = WAITING;
    
    // WARNING: The sizes of these buffers are very close to their limits
    // Exercise extreme caution if making any modifications to avoid overflows

    char json[300]; //for while 254 caracteres
    char loraPacket[300]; //for while 111 caracteres
    char csv[300]; //for while 243carcteres

    void buildJson(bool imuValid);
    void buildCsv(bool imuValid);

    const char* stateToString() const;
};