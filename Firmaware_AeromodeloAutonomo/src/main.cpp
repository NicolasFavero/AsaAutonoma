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
#include "DataTypes.h"
#include "PID.h"
//=======================================================================================================================================================
//===================================================================DEFINITIONS=========================================================================
//OTHERS
unsigned long oldMillis = millis();

//BATTERY
bool powerMode = false;
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
//=================================================================DEFINITIONS===========================================================================
//========================================================================================================================================================
//==================================================================CLASSES==============================================================================
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
LoRa lora(CS_RadioPin, DIO0_RadioPin, RESET_RadioPin, FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SYNC_WORD, POWER, PREAMBLE_LENGTH);
Telemetry telemetry(gps, bmp, ads);

QueueHandle_t attitudeQueue;
QueueHandle_t navigationQueue;
//==================================================================CLASSES===============================================================================
//========================================================================================================================================================
//=================================================================PROTOTYPES=============================================================================
bool isLowPowerModeEnabled();
void taskControl(void *pv);
void taskTelemetry(void *pv);
void PID(ServoPositions& angle, bool gpsData = false);
//=================================================================PROTOTYPES=============================================================================
//=========================================================================================================================================================
//===================================================================SETUP=================================================================================
void setup(){
  Serial.begin(115200);
  Serial.println("Beginning...");
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000); 

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);

  led.white();
  //led.white();

  while(!imu.begin()){led.red();}
  while(!bmp.begin()){led.red();}
  while(!gps.begin()){led.red();}
  while(!ads.begin()){led.red();}
  while(!lora.begin()){led.red();}

  while(!elevator.begin()){led.red();}
  while(!leftAlieron.begin()){led.red();}
  while(!rightAlieron.begin()){led.red();}


  while(!sd.begin()) {Serial.println("SD FAIL"); led.red();}
  while(!sd.createFile()) {Serial.println("ERRO CRIAR ARQUIVO"); led.red();}
  while(!sd.saveLine("tempo_ms,altitude,pitch,roll,yaw")){Serial.println("Erro aao criar Header"); led.red();}

  powerMode = isLowPowerModeEnabled();

  attitudeQueue = xQueueCreate(1, sizeof(AttitudeData));
  navigationQueue = xQueueCreate(1, sizeof(NavigationData));

  if(attitudeQueue == NULL || navigationQueue == NULL){
    while(true){
      led.red();
      delay(500);
    }
  }

  led.green();

  xTaskCreatePinnedToCore(
    taskControl,
    "Control",
    10000,
    NULL,
    2,
    NULL,
    0
  );
  xTaskCreatePinnedToCore(
    taskTelemetry,
    "Data",
    10000,
    NULL,
    1,
    NULL,
    1
  );

}
//===================================================================SETUP==================================================================================
//==========================================================================================================================================================
//================================================================KILLING TASK==============================================================================
void loop(){vTaskDelete(NULL);}
//================================================================KILLING TASK==============================================================================
//==========================================================================================================================================================
//=================================================================TASK AT CORE 0===========================================================================
void taskControl(void *pv){

  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(5); //200HZ

  while(true){
      bool useNewValues = true;
  //=====================FOR SEND================
    static AttitudeData attitudeData;

    attitudeData.pitch = imu.getPitch();
    attitudeData.roll = imu.getRoll();
    attitudeData.yaw = imu.getYaw();

    attitudeData.accX = imu.getAccX();
    attitudeData.accY = imu.getAccY();
    attitudeData.accZ = imu.getAccZ();

    attitudeData.gyroX = imu.getGyroX();
    attitudeData.gyroY = imu.getGyroY();
    attitudeData.gyroZ = imu.getGyroZ();

    attitudeData.magX= imu.getMagX();
    attitudeData.magY = imu.getMagY();
    attitudeData.magZ = imu.getMagZ();

    xQueueOverwrite(attitudeQueue, &attitudeData);
  //=====================FOR SEND================
  //====================TO  RECEIVE==============
    static NavigationData nav;

    if(xQueueReceive(navigationQueue,&nav,0)){/*PID.updateNavigation(nav)*/; useNewValues = true;}
  //====================TO  RECEIVE==============

    static ServoPositions angle;

    imu.update();

    //PID(angle, attitudeData, nav, useNewValues); //updates the angles;

    elevator.write(angle.elevator);
    leftAlieron.write(angle.leftAlieron);
    rightAlieron.write(angle.rightAlieron);


    vTaskDelayUntil(&lastWake,period);
  }
}
//=================================================================TASK AT CORE 0===========================================================================
//==========================================================================================================================================================
//=================================================================TASK AT CORE 1===========================================================================
void taskTelemetry(void *pv){

  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(100); //10HZ

  while(true){
    bool isImuValid = false;

    bmp.update();
    gps.update();

    //====================TO  RECEIVE==============
    static AttitudeData attitude;
    if(xQueueReceive(attitudeQueue, &attitude, 0)){telemetry.setAttitudeData(attitude); isImuValid = true;}
    //====================TO  RECEIVE==============

    telemetry.update(isImuValid);
    powerMode = isLowPowerModeEnabled();

    //=====================FOR SEND================
    static NavigationData navigationData;

    bool gpsValid = gps.isValid();

    navigationData.latitude = gpsValid ? gps.getLatitude() : 0;
    navigationData.longitude = gpsValid ? gps.getLongitude() : 0;
    navigationData.gpsAltitude = gpsValid ? gps.getAltitude() : 0;
    navigationData.course = gpsValid ? gps.getCourse() : 0;

    navigationData.baroAltitude = bmp.getRawAltitude();
    navigationData.temperature = bmp.getTemperature();

    navigationData.battery = ads.batteryLevel();

    xQueueOverwrite(navigationQueue, &navigationData);
    //=====================FOR SEND================

    //it runs when the BATTERY_LEVEL its HIGH
    if(!powerMode){

      if(millis() - oldMillis > 1E3){

        lora.send(telemetry.getLoraPacket(powerMode));
        sd.saveLine(telemetry.getCsv());

        oldMillis = millis();
      }
    }
    //it runs when the BATTERY_LEVEL its LOW
      else{        
        if(millis() - oldMillis > 2E4){
          
          lora.send(telemetry.getLoraPacket(powerMode));
        
          oldMillis = millis();
        }
      }
      vTaskDelayUntil(&lastWake, period);
  }
}
//=================================================================TASK AT CORE 1===========================================================================
//==========================================================================================================================================================
//================================================================OTHERS FUNCTIONS==========================================================================
bool isLowPowerModeEnabled(){

    if(ads.batteryLevel() < BATTERY_LIMIT){return true;}
    else{return false;}
  }