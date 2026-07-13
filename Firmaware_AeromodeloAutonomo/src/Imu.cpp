#include "Imu.h"
#include <Arduino.h>
#include "math.h"
//Wire.begin(SDA, SCL)  --> main.cpp

IMU::IMU():imu() {}

bool IMU::begin() {

    imu.begin(Wire, ADDRESS);

    if (imu.status != ICM_20948_Stat_Ok) {
        Serial.println("Erro Critico: ICM-20948 nao respondeu em 0x68");
        while (1);
    }

    bool success = true;

    success &= (imu.initializeDMP() == ICM_20948_Stat_Ok);
    success &= (imu.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
    success &= (imu.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok);

    success &= (imu.enableFIFO() == ICM_20948_Stat_Ok);
    success &= (imu.enableDMP() == ICM_20948_Stat_Ok);
    success &= (imu.resetFIFO() == ICM_20948_Stat_Ok);
    success &= (imu.resetDMP() == ICM_20948_Stat_Ok);

    if (!success) {
        Serial.println("Erro: Falha ao configurar as funcoes do DMP!");
        return false;
    }

    Serial.println("Filtro de Kalman (DMP) pronto. Movimente o ICM20948...");
    return true;
}

bool IMU::update(){

    if (!imu.dataReady()) {
        return false;
    }

    imu.getAGMT();

    accX = imu.accX();
    accY = imu.accY();
    accZ = imu.accZ();

    gyroX = imu.gyrX();
    gyroY = imu.gyrY();
    gyroZ = imu.gyrZ();

    magX = imu.magX();
    magY = imu.magY();
    magZ = imu.magZ();


    icm_20948_DMP_data_t dmpData;
    imu.readDMPdataFromFIFO(&dmpData);

    if ((imu.status != ICM_20948_Stat_Ok) &&
        (imu.status != ICM_20948_Stat_FIFOMoreDataAvail)) {

        if (imu.status == ICM_20948_Stat_FIFOIncompleteData) {
            imu.resetFIFO();
        }

        return false;
    }

    if (!(dmpData.header & DMP_header_bitmap_Quat6) &&
        !(dmpData.header & DMP_header_bitmap_Quat9)) {
        return false;
    }

    double q1, q2, q3;

    if (dmpData.header & DMP_header_bitmap_Quat9) {
        q1 = ((double)dmpData.Quat9.Data.Q1) / 1073741824.0;
        q2 = ((double)dmpData.Quat9.Data.Q2) / 1073741824.0;
        q3 = ((double)dmpData.Quat9.Data.Q3) / 1073741824.0;
    } else {
        q1 = ((double)dmpData.Quat6.Data.Q1) / 1073741824.0;
        q2 = ((double)dmpData.Quat6.Data.Q2) / 1073741824.0;
        q3 = ((double)dmpData.Quat6.Data.Q3) / 1073741824.0;
    }

    double q0_sq = 1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3));
    double q0 = (q0_sq > 0.0) ? sqrt(q0_sq) : 0.0;

    float raw_1 = atan2(
        2.0 * (q0 * q1 + q2 * q3),
        1.0 - 2.0 * (q1 * q1 + q2 * q2)
    ) * 180.0f / M_PI;

    double temp = 2.0 * (q0 * q2 - q3 * q1);

    if (temp > 1.0) temp = 1.0;
    if (temp < -1.0) temp = -1.0;

    float raw_2 = asin(temp) * 180.0f / M_PI;

    float yaw_ = atan2(
        2.0 * (q0 * q3 + q1 * q2),
        1.0 - 2.0 * (q2 * q2 + q3 * q3)
    ) * 180.0f / M_PI;

    roll  = -raw_2;
    pitch = raw_1;
    yaw   = yaw_;

    return true;
}

void IMU::print(){
    Serial.print("ROLL:");
    Serial.print(roll, 1);
    Serial.print(",");

    Serial.print("PITCH:");
    Serial.print(pitch, 1);
    Serial.print(",");

    Serial.print("YAW:");
    Serial.println(yaw, 1);
}

float IMU::getPitch() const {return pitch;}
float IMU::getRoll() const {return roll;}
float IMU::getYaw() const {return yaw;}

float IMU::getAccX() const {return accX;}
float IMU::getAccY() const {return accY;}
float IMU::getAccZ() const {return accZ;}

float IMU::getGyroX() const {return gyroX;}
float IMU::getGyroY() const {return gyroY;}
float IMU::getGyroZ() const {return gyroZ;}

float IMU::getMagX() const {return magX;}
float IMU::getMagY() const {return magY;}
float IMU::getMagZ() const {return magZ;}

const char* IMU::packet(){

    snprintf(
        packetBuffer,
        sizeof(packetBuffer),
        "%.1f,%.1f,%.1f",
        roll,
        pitch,
        yaw
    );

    return packetBuffer;
}