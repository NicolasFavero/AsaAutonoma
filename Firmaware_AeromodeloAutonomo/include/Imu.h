#pragma once
//  OBS CONFERIR SE OS EIXOS DO MAGNOMETROS JÁ ESTÃO SENDO CORRIGIDOS NA LIB ICM20948 OU SE ESTOU CERTO NO CÓDIGO, O YAW ESTÁ RUIM AINDA
#include "Wire.h"
#include "ICM_20948.h"
#include <Arduino.h>

//Wire.begin(SDA, SCL)  --> main.cpp

class IMU{
    public:

        IMU();

        bool begin();
        bool update();
        void print();
        const char* packet();

        float getPitch() const;
        float getRoll() const;
        float getYaw() const;

        float getAccX() const;
        float getAccY() const;
        float getAccZ() const;

        float getGyroX() const;
        float getGyroY() const;
        float getGyroZ() const;

        float getMagX() const;
        float getMagY() const;
        float getMagZ() const;

    private:
        ICM_20948_I2C imu;

        float pitch = 0.0f;
        float roll  = 0.0f;
        float yaw   = 0.0f; 

        float accX = 0.0f;
        float accY = 0.0f;
        float accZ = 0.0f;

        float gyroX = 0.0f;
        float gyroY = 0.0f;
        float gyroZ = 0.0f;

        float magX = 0.0f;
        float magY = 0.0f;
        float magZ = 0.0f;
        
        char packetBuffer[32];
        unsigned long lastUpdateMicros = 0;

        static constexpr uint8_t ADDRESS = 0; //it corresponds with the 0x68

};


//só pra lembrar enquanto to programando
 // Wire.begin(SDA_PIN, SCL_PIN);
 // Wire.setClock(400000); 