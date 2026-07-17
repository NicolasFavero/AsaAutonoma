#pragma once

#include <Arduino.h>
#include "DataTypes.h"

class PID{

    public:

        PID(){};
        
        void setAngles(
            ServoPositions& angle,
            AttitudeData& attitudeData,
            NavigationData& nav,
            bool gpsData = false
        );


    private:

        float lastErrorPitch = 0.0f;
        float lastErrorRoll = 0.0f;


        float integralPitch = 0.0f;
        float integralRoll = 0.0f;


        uint32_t lastTime = 0;


        float KpPitch = 1.0f;
        float KiPitch = 0.1f;
        float KdPitch = 0.1f;


        float KpRoll = 1.0f;
        float KiRoll = 0.1f;
        float KdRoll = 0.1f;

        float integralLimit = 100.0f;
};