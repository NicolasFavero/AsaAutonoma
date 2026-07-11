#pragma once

#include "Wire.h"
#include <ICM20948_WE.h>
#include <MadgwickAHRS.h>

#define SDA_PIN 3
#define SCL_PIN 2

#define FILTER_UPDATE_HZ 100 
constexpr unsigned long FILTER_PERIOD_MICROS = 1000000 / FILTER_UPDATE_HZ;

// ==========================================
// INSIRA AQUI OS SEUS OFFSETS DA CALIBRAÇÃO:
// ==========================================
#define MAG_X_OFFSET  -38.87f
#define MAG_Y_OFFSET   0.22f
#define MAG_Z_OFFSET  38.27f

#define GYRO_X_OFFSET -0.064267f
#define GYRO_Y_OFFSET  0.792195f
#define GYRO_Z_OFFSET -0.226051f



class IMU{
    public:

        IMU():

        float getPitch();
        float getRoll();
        float getYaw();

        void begin();
        void update();
        void printValues();
        char* packet();


    private:
        ICM20948_WE imu;
        Madgwick filter;

        float pitch;
        float roll;
        float yaw;
        char* packet[32];
        unsigned long lastUpdateMicros = 0;


}


//só pra lembrar enquanto to programando
 // Wire.begin(SDA_PIN, SCL_PIN);
 // Wire.setClock(400000); 
//ICM20948_WE imu(0x68);
//Madgwick filter(0.14f); // Seu beta escolhido e testado de 0.14f
