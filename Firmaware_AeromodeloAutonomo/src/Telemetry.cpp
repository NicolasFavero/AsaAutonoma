#include "Telemetry.h"
#include <Arduino.h>

Telemetry::Telemetry(GPS& gps, BMP280& bmp, ADS& ads)
    : gps(gps), bmp(bmp), ads(ads)
{

}
void Telemetry::setAttitudeData(const AttitudeData& data)
{
    attitude = data;
}
void Telemetry::update(bool imuValid)
{
    buildJson(imuValid);
    buildCsv(imuValid);
}
void Telemetry::buildJson(bool imuValid)
{

    bool gpsOk = gps.isValid();

    //float pitch = imuValid ? attitude.pitch : 0.0f;
    float pitch = attitude.pitch;
    float roll  = imuValid ? attitude.roll  : 0.0f;
    float yaw   = imuValid ? attitude.yaw   : 0.0f;

    float lat = gpsOk ? gps.getLatitude() : 0.0;
    float lon = gpsOk ? gps.getLongitude() : 0.0;
    float gpsAlt = gpsOk ? gps.getAltitude() : 0.0;
    float course = gpsOk ? gps.getCourse() : 0.0;

    snprintf(
        json,
        sizeof(json),
        "{\"state\":\"%s\","
        "\"gpsOk\":%d,"
        "\"imuOk\":%d,"
        "\"pitch\":%.2f,"
        "\"roll\":%.2f,"
        "\"yaw\":%.2f,"
        "\"bmpAlt\":%.2f,"
        "\"gpsAlt\":%.2f,"
        "\"lat\":%.6f,"
        "\"lon\":%.6f,"
        "\"course\":%.2f,"
        "\"temp\":%.2f,"
        "\"date\":\"%02d/%02d/%02d\","
        "\"time\":\"%02d:%02d:%02d\","
        "\"sats\":%d,"
        "\"bat\":%.2f}",

        stateToString(),

        gpsOk,
        imuValid,
        pitch,
        roll,
        yaw,

        bmp.getRawAltitude(),
        gpsAlt,

        lat,
        lon,

        course,

        bmp.getTemperature(),

        gps.getDay(),
        gps.getMonth(),
        gps.getYear() % 100,

        gps.getHour(),
        gps.getMinute(),
        gps.getSecond(),

        gps.getSatellites(),

        ads.batteryLevel()
    );

}
void Telemetry::buildCsv(bool imuValid)
{

    bool gpsOk = gps.isValid();

    float pitch = imuValid ? attitude.pitch : 0.0f;
    float roll  = imuValid ? attitude.roll  : 0.0f;
    float yaw   = imuValid ? attitude.yaw   : 0.0f;

    float accX = imuValid ? attitude.accX : 0.0f;
    float accY = imuValid ? attitude.accY : 0.0f;
    float accZ = imuValid ? attitude.accZ : 0.0f;

    float gyroX = imuValid ? attitude.gyroX : 0.0f;
    float gyroY = imuValid ? attitude.gyroY : 0.0f;
    float gyroZ = imuValid ? attitude.gyroZ : 0.0f;

    float magX = imuValid ? attitude.magX : 0.0f;
    float magY = imuValid ? attitude.magY : 0.0f;
    float magZ = imuValid ? attitude.magZ : 0.0f;

    float lat = gpsOk ? gps.getLatitude() : 0.0;
    float lon = gpsOk ? gps.getLongitude() : 0.0;
    float gpsAlt = gpsOk ? gps.getAltitude() : 0.0;
    float course = gpsOk ? gps.getCourse() : 0.0;

    snprintf(
        csv,
        sizeof(csv),

        "%lu,"
        "%s,"
        "%d,"
        "%d,"
        "%d,"
        "%02d/%02d/%02d,"
        "%02d:%02d:%02d,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.6f,"
        "%.6f,"
        "%.2f,"
        "%.2f",

        millis(),

        stateToString(),

        gpsOk,
        imuValid,

        gps.getSatellites(),

        gps.getDay(),
        gps.getMonth(),
        gps.getYear() % 100,

        gps.getHour(),
        gps.getMinute(),
        gps.getSecond(),

        pitch,
        roll,
        yaw,

        accX,
        accY,
        accZ,

        gyroX,
        gyroY,
        gyroZ,

        magX,
        magY,
        magZ,

        bmp.getRawAltitude(),
        gpsAlt,

        bmp.getTemperature(),

        lat,
        lon,

        course,

        ads.batteryLevel()
    );

}
const char* Telemetry::getLoraPacket(bool reduced)
{
    if(!reduced)
    {
        strcpy(loraPacket, json);
        return loraPacket;
    }


    bool gpsOk = gps.isValid();

    float lat = gpsOk ? gps.getLatitude() : 0.0;
    float lon = gpsOk ? gps.getLongitude() : 0.0;
    float alt = gpsOk ? gps.getAltitude() : 0.0;
    float course = gpsOk ? gps.getCourse() : 0.0;


    snprintf(
        loraPacket,
        sizeof(loraPacket),

        "{\"s\":\"%s\",\"g\":%d,\"alt\":%.2f,\"lat\":%.6f,\"lon\":%.6f,\"c\":%.2f,\"bat\":%.2f}",

        stateToString(),
        gpsOk,
        alt,
        lat,
        lon,
        course,
        ads.batteryLevel()
    );


    return loraPacket;
}
const char* Telemetry::getCsvHeader() const {

    return
    "millis,"
    "state,"
    "gpsValid,"
    "imuValid,"
    "sats,"
    "date,"
    "time,"
    "pitch,"
    "roll,"
    "yaw,"
    "accX,"
    "accY,"
    "accZ,"
    "gyroX,"
    "gyroY,"
    "gyroZ,"
    "magX,"
    "magY,"
    "magZ,"
    "baroAlt,"
    "gpsAlt,"
    "temp,"
    "lat,"
    "lon,"
    "course,"
    "battery";

}
const char* Telemetry::getJson() const {return json;}
const char* Telemetry::getCsv() const {return csv;}
void Telemetry::setState(FlightState newState) {state = newState;}

const char* Telemetry::stateToString() const
{
    switch(state)
    {
        case WAITING:
            return "WAITING";

        case FLYING:
            return "FLYING";

        case LANDED:
            return "LANDED";

        default:
            return "UNKNOWN";
    }
}