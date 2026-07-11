#include <Arduino.h>
#include "IMU.h"

static constexpr uint8_t SDA_PIN = 3; 
static constexpr uint8_t SCL_PIN = 2;


IMU imu; 

void setup(){
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000); 

  
  while(imu.begin()){}

}

void loop(){}