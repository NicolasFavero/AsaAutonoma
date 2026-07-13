#include "Arduino.h"
#include "IMU.h"
#include "BMP280.h"
#include "GPS.h"
#include "Led.h"
#include "Servo.h"
#include "SdLogger.h"
#include "RF96W.h"
#include "ADS1X15.h"
#include "Telemetry.h"
//  OBS CONFERIR SE OS EIXOS DO MAGNOMETROS JÁ ESTÃO SENDO CORRIGIDOS NA LIB ICM20948 OU SE ESTOU CERTO NO CÓDIGO, O YAW ESTÁ RUIM AINDA
// ou yaw pelo gps

static constexpr float BATTERY_LIMIT = 6.0;

//I2C
static constexpr uint8_t SDA_PIN = 3; 
static constexpr uint8_t SCL_PIN = 2;

//Servos
static constexpr uint8_t elevatorPin = 12;
static constexpr uint8_t leftAlieronPin = 11;
static constexpr uint8_t rightAlieronPin = 10;
//static constexpr uint8_t rudderPin = 43;

static constexpr uint8_t elevatorChannel = 0;
static constexpr uint8_t leftAlieronChannel = 1;
static constexpr uint8_t rightAlieronChannel = 2;
//static constexpr uint8_t rudderChannel = 3;

static constexpr uint8_t servoFrequency = 50;
static constexpr uint8_t servoResolution = 14;

//SPI
static constexpr uint8_t SCK_PIN = 5;
static constexpr uint8_t MOSI_PIN = 6;
static constexpr uint8_t MISO_PIN = 7;

//SD
static constexpr uint8_t CS_SdPin = 1;

//LoRa
static constexpr uint8_t CS_RadioPin = 4;
static constexpr uint8_t DIO0_RadioPin = 8;
static constexpr uint8_t RESET_RadioPin = 9;
//static constexpr uint8_t BUSY_RadioPin = 44;

static constexpr float FREQUENCY = 915.0f;
static constexpr float BANDWIDTH = 125.0f;
static constexpr uint8_t SPREADING_FACTOR = 10;
static constexpr uint8_t CODING_RATE = 5;
static constexpr uint8_t SYNC_WORD = 0x12;
static constexpr int8_t POWER = 20;
static constexpr uint16_t PREAMBLE_LENGTH = 8;

//GPS
static constexpr uint8_t GPS_TX = 13;

//========================================================================
ADS ads;
IMU imu;
BMP280 bmp;
Led led;
GPS gps(GPS_TX);
SdLogger sd(CS_SdPin);
Servo elevator(elevatorPin, elevatorChannel, servoFrequency, servoResolution);
Servo leftAlieron(leftAlieronPin, leftAlieronChannel, servoFrequency, servoResolution);
Servo rightAlieron(rightAlieronPin, rightAlieronChannel, servoFrequency, servoResolution);
//Servo rudder(rudderPin, rudderChannel, servoFrequency, servoResolution);
LoRa lora(
    CS_RadioPin,
    DIO0_RadioPin,
    RESET_RadioPin,

    FREQUENCY,
    BANDWIDTH,
    SPREADING_FACTOR,
    CODING_RATE,
    SYNC_WORD,
    POWER,
    PREAMBLE_LENGTH
);
Telemetry telemetry(imu, gps, bmp, ads);
//=======================================================================
  bool isLowPowerModeEnabled(){
    if(ads.batteryLevel() < BATTERY_LIMIT){return true;}
    else{return false;}
  }
  bool powerMode = isLowPowerModeEnabled();
//=======================================================================

void setup(){
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000); 

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);

  led.white();
  //led.white();

  //while(!imu.begin()){led.red();}
  //while(!bmp.begin()){led.red();}
  //while(!gps.begin()){led.red();}
  //while(!ads.begin()){led.red();}
  while(!lora.begin()){led.red();}

  //while(!elevator.begin()){led.red();}
  //while(!leftAlieron.begin()){led.red();}
  //while(!rightAlieron.begin()){led.red();}


  while(!sd.begin()) {Serial.println("SD FAIL"); led.red();}
  while(!sd.createFile()) {Serial.println("ERRO CRIAR ARQUIVO"); led.red();}
  while(!sd.saveLine("tempo_ms,altitude,pitch,roll,yaw")){Serial.println("Erro aao criar Header"); led.red();}

  led.green();

}

unsigned long oldMillis = millis();

void loop(){

  gps.update()? (void)0 : led.blue();
  bmp.update()? (void)0 : led.red();
  imu.update()? (void)0 : led.red();

  powerMode = isLowPowerModeEnabled();
  telemetry.update();

  if(!powerMode){
    if(millis() - oldMillis > 1E3){

      //bmp.print();
      //imu.print();
      Serial.println(telemetry.getJson());
      lora.send(telemetry.getLoraPacket(powerMode));
      sd.saveLine(telemetry.getCsv());
    }
  }
    else{
      if(millis() - oldMillis > 1E4){
        
        lora.send(telemetry.getLoraPacket(powerMode));
        oldMillis = millis();
      }
    }

}





