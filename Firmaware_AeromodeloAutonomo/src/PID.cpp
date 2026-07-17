#include "PID.h"
#include "DataTypes.h"

void PID::setAngles(ServoPositions& angle, AttitudeData& attitudeData, NavigationData& nav, bool gpsData){

    //200Hz, choosed in taskControl in main.cpp

    const float pitchTarget = -3.0f;
    const float rollTarget = 0.0f;

    const float neutral = 90.0f;


    uint32_t now = micros();


    float dt;

    if(lastTime == 0)
    {
        dt = 0.005f; // primeiro ciclo assume 200Hz (5ms)
    }
    else
    {
        dt = (now - lastTime) / 1000000.0f;
    }

    lastTime = now;

    //================ PITCH =================

    float errorPitch = pitchTarget - attitudeData.pitch;

    integralPitch += errorPitch * dt;

    if(integralPitch > integralLimit)
        integralPitch = integralLimit;

    if(integralPitch < -integralLimit)
        integralPitch = -integralLimit;

    float derivativePitch =
        (errorPitch - lastErrorPitch) / dt;

    float outputPitch =
        (KpPitch * errorPitch) +
        (KiPitch * integralPitch) +
        (KdPitch * derivativePitch);


    lastErrorPitch = errorPitch;

    //================ ROLL =================
    float errorRoll = rollTarget - attitudeData.roll;

    integralRoll += errorRoll * dt;

    if(integralRoll > integralLimit)
        integralRoll = integralLimit;

    if(integralRoll < -integralLimit)
        integralRoll = -integralLimit;


    float derivativeRoll =
        (errorRoll - lastErrorRoll) / dt;

    float outputRoll =
        (KpRoll * errorRoll) +
        (KiRoll * integralRoll) +
        (KdRoll * derivativeRoll);

    lastErrorRoll = errorRoll;

    //================ SERVO =================
    angle.elevator =constrain(neutral + outputPitch, 0, 180);
    angle.leftAlieron =constrain(neutral + outputRoll, 0, 180);
    angle.rightAlieron = constrain(neutral + outputRoll, 0 ,180);

}