#include "IMU.h"
#include <Arduino.h>

IMU::IMU(static constexpr adress, static constexpr float beta):imu(adress), filter(beta){}
void IMU::begin(){

      if (!imu.init()) {
    Serial.println("ERRO ICM20948");
    while (1);
  }
  imu.initMagnetometer();
  imu.setAccRange(ICM20948_ACC_RANGE_2G);
  imu.setGyrRange(ICM20948_GYRO_RANGE_250);
  imu.setAccDLPF(ICM20948_DLPF_4);
  imu.setGyrDLPF(ICM20948_DLPF_4);

  filter.begin(FILTER_UPDATE_HZ);
  lastUpdateMicros = micros();
}
void IMU::update(){
    
  unsigned long currentMicros = micros();

  if (currentMicros - lastUpdateMicros >= FILTER_PERIOD_MICROS) {
    lastUpdateMicros += FILTER_PERIOD_MICROS; 

    imu.readSensor();

    xyzFloat acc;
    xyzFloat gyro;
    xyzFloat mag; 

    imu.getGValues(&acc);
    imu.getGyrValues(&gyro);
    imu.getMagValues(&mag); 

    // 1) Giroscópio e Acelerômetro brutos
    float gx_raw = gyro.x + GYRO_X_OFFSET;
    float gy_raw = gyro.y + GYRO_Y_OFFSET;
    float gz_raw = gyro.z + GYRO_Z_OFFSET;
   
    float ax_raw = acc.x;
    float ay_raw = acc.y;
    float az_raw = acc.z;

    // 2) Calibração de Hard Iron + Alinhamento do Silício Mag vs Accel
    float mx_cal = (mag.y) - MAG_Y_OFFSET;
    float my_cal = (mag.x) - MAG_X_OFFSET;
    float mz_cal = (-mag.z) - MAG_Z_OFFSET;

    // 3) Correção geométrica final (Orientação do seu Aeromodelo)
    float gx = -gy_raw;
    float gy = gx_raw;
    float gz = gz_raw;

    float ax = -ay_raw;
    float ay = ax_raw;
    float az = az_raw;

    float mx = -my_cal;
    float my = mx_cal;
    float mz = mz_cal;

    // 4) Fusão de 9 Eixos estável
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

    pitch = filter.getRoll();
    roll = filter.getRoll();
    yaw= filter.getYaw();

    }
}
void IMU::printValues(){
    Serial.print("ROLL:");  Serial.print(roll, 1);  Serial.print(",");
    Serial.print("PITCH:"); Serial.print(pitch, 1); Serial.print(",");
    Serial.print("YAW:");   Serial.println(yaw, 1);
}
float IMU::getPitch(){return pitch;}
float IMU::getRoll(){return roll;}
float IMU::getYaw(){return yaw;}
char* IMU::packet(){
    
    snprintf(
    packet,
    sizeof(packet),
    "%.1f,%.1f,%.1f",
    roll,
    pitch,
    yaw);

    return packet;
}