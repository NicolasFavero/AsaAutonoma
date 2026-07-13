#include "Telemetry.h"
#include <Arduino.h>


Telemetry::Telemetry(IMU& imu, GPS& gps, BMP280& bmp, ADS& ads)
    : imu(imu), gps(gps), bmp(bmp), ads(ads)
{

}


void Telemetry::update()
{
    buildJson();
    buildCsv();
}


void Telemetry::buildJson()
{

    bool gpsOk = gps.isValid();


    float lat = gpsOk ? gps.getLatitude() : 0.0;
    float lon = gpsOk ? gps.getLongitude() : 0.0;
    float gpsAlt = gpsOk ? gps.getAltitude() : 0.0;
    float course = gpsOk ? gps.getCourse() : 0.0;


    snprintf(
        json,
        sizeof(json),
        "{\"state\":\"%s\","
        "\"gpsValid\":%d,"
        "\"pitch\":%.2f,"
        "\"roll\":%.2f,"
        "\"yaw\":%.2f,"
        "\"baroAlt\":%.2f,"
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

        imu.getPitch(),
        imu.getRoll(),
        imu.getYaw(),

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



void Telemetry::buildCsv()
{

    bool gpsOk = gps.isValid();


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

        gps.getSatellites(),

        gps.getDay(),
        gps.getMonth(),
        gps.getYear() % 100,

        gps.getHour(),
        gps.getMinute(),
        gps.getSecond(),


        imu.getPitch(),
        imu.getRoll(),
        imu.getYaw(),


        imu.getAccX(),
        imu.getAccY(),
        imu.getAccZ(),


        imu.getGyroX(),
        imu.getGyroY(),
        imu.getGyroZ(),


        imu.getMagX(),
        imu.getMagY(),
        imu.getMagZ(),


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

    bool gpsOk = gps.isValid();


    float lat = gpsOk ? gps.getLatitude() : 0.0;
    float lon = gpsOk ? gps.getLongitude() : 0.0;
    float gpsAlt = gpsOk ? gps.getAltitude() : 0.0;
    float course = gpsOk ? gps.getCourse() : 0.0;



    if(reduced)
    {

        snprintf(
            loraPacket,
            sizeof(loraPacket),

            "{\"state\":\"%s\","
            "\"gpsValid\":%d,"
            "\"alt\":%.2f,"
            "\"lat\":%.6f,"
            "\"lon\":%.6f,"
            "\"course\":%.2f,"
            "\"bat\":%.2f}",

            stateToString(),

            gpsOk,

            gpsAlt,

            lat,
            lon,

            course,

            ads.batteryLevel()
        );

    }
    else
    {

        snprintf(
            loraPacket,
            sizeof(loraPacket),

            "%s",
            json
        );

    }


    return loraPacket;

}



const char* Telemetry::getCsvHeader() const
{

    return
    "millis,"
    "state,"
    "gpsValid,"
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



const char* Telemetry::getJson() const
{
    return json;
}



const char* Telemetry::getCsv() const
{
    return csv;
}



void Telemetry::setState(FlightState newState)
{
    state = newState;
}



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