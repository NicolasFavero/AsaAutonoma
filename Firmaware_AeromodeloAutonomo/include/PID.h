#pragma once

#include <Arduino.h>
#include <DataTypes.h>



class PID {

    public:
        PID(){};

        void setAngles(ServoPositions& angle, AttitudeData& attitudeData, NavigationData& nav, bool gpsData);

        struct PID_variables{
            // Antes sem valor inicial nenhum -- kp/ki/kd/integralLimit
            // ficavam com lixo de memoria ate a primeira config ser
            // aplicada. Zerado por seguranca: com ganho 0 o PID nao
            // produz nenhuma correcao ate ser configurado de verdade.
            float kp = 0.0f;
            float ki = 0.0f;
            float kd = 0.0f;

            float lastError = 0.0f;
            float error = 0.0f;
            float target = 0.0f;

            float proporcional = 0.0f;
            float integral = 0.0f;
            float integralLimit = 0.0f;
            float derivative = 0.0f;

            float output = 0.0f;
        };

        PID_variables pitch;
        PID_variables roll;
        PID_variables yaw; // Ainda nao usado em computePID, so guarda o valor pro futuro.

    private:

        struct ServoParamters{

            float setAngle = 90.0f;
            float safeAngle = 90.0f;
            float targetAngle = 90.0f;
            float neutralAngle = 90.0f;
        };

        ServoParamters elevator;
        ServoParamters leftAileron;
        ServoParamters rightAileron;
        // Histórico de tempo
        uint32_t lastTime = 0;

        void computePID(PID_variables& pid, ServoParamters& servo, float currentAngle, float dt, float integralLimit, float maxMovement, int minLimit, int maxLimit);

};
