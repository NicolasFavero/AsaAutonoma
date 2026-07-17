// Arduino
#include "Arduino.h"

// Config
#include "Pins.h"
#include "ServomotorConfig.h"
#include "DataTypes.h"
#include "CurrentConfig.h"

// Drivers
#include "IMU.h"
#include "GPS.h"
#include "BMP280.h"
#include "ADS1X15.h"

// Communication
#include "RF96W.h"
#include "WiFiAP.h"

// Storage
#include "SdLogger.h"
#include "PreferencesManager.h"

// Control
#include "PID.h"
#include "Servo.h"

// Telemetry
#include "Telemetry.h" //ALTEREI PRA DEBUG     float pitch = imuValid ? attitude.pitch : 0.0f; LEMBRAR DE VOLTAR ATRAS DEPOIS!!!!!!!!!!!!!!!

// Debug
#include "Led.h"
//=======================================================================================================================================================
//OTHERS
unsigned long lastTelemetryMillis = 0;
volatile bool newOffsetsAvailable = false;

//==================================================================STRUCTS===============================================================================
ImuOffsets currentOffsets; 
SystemConfig systemConfig;
SystemStatus systemStatus;
//========================================================================================================================================================
//==================================================================CLASSES==============================================================================
// Drivers
IMU imu;
ADS ads;
BMP280 bmp;
GPS gps(Pins::GPS_TX);

// Communication
LoRa lora(Pins::LORA_CS, Pins::LORA_DIO0, Pins::LORA_RESET, systemConfig.lora);
WifiAP wifi;

// Storage
SdLogger sd(Pins::SD_CS);
PreferencesManager nvs;

// Control
PID pid;

// Actuators
Servo elevator    (Pins::ELEVATOR,      ServoConfig::Channel::ELEVATOR,      ServoConfig::FREQUENCY, ServoConfig::RESOLUTION);
Servo leftAlieron (Pins::LEFT_AILERON,  ServoConfig::Channel::LEFT_AILERON,  ServoConfig::FREQUENCY, ServoConfig::RESOLUTION);
Servo rightAlieron(Pins::RIGHT_AILERON, ServoConfig::Channel::RIGHT_AILERON, ServoConfig::FREQUENCY, ServoConfig::RESOLUTION);
//Servo rudder    (Pins::RUDDER,        ServoConfig::Channel::RUDDER,        ServoConfig::FREQUENCY, ServoConfig::RESOLUTION); 

// Telemetry
Telemetry telemetry(gps, bmp, ads);

//Visual Debug
Led led;

//========================================================================================================================================================
//==============================================================FREEERTOS QUEUES===============================================================================
QueueHandle_t attitudeQueue;
QueueHandle_t navigationQueue;
portMUX_TYPE offsetMux = portMUX_INITIALIZER_UNLOCKED;
//========================================================================================================================================================
//=================================================================PROTOTYPES=============================================================================
// TASKS
void taskControl(void *pv);
void taskData(void *pv);

// TASK CONTROL HELPERS
bool readAttitudeData(AttitudeData& attitude);

// TASK DATA HELPERS
void sendNavigationData(NavigationData& navigationData);
void handleOutputs();
void handleWifi(const AttitudeData& attitude, const NavigationData& navigationData);

// GENERAL HELPERS
bool isLowPowerModeEnabled();

//=========================================================================================================================================================
//===================================================================SETUP=================================================================================
void setup(){
  Serial.begin(115200);
  Serial.println("Beginning...");
  Wire.begin(Pins::SDA, Pins::SCL);
  Wire.setClock(400000); 

  SPI.begin(Pins::SCK, Pins::MISO, Pins::MOSI);

  led.white();
  //led.white();

  nvs.loadSystem(systemConfig);
  nvs.loadOffsets(currentOffsets);

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
  while(!sd.saveLine(telemetry.getCsvHeader())){Serial.println("Erro aao criar Header"); led.red();}


  attitudeQueue = xQueueCreate(1, sizeof(AttitudeData));
  navigationQueue = xQueueCreate(1, sizeof(NavigationData));

  if(attitudeQueue == NULL || navigationQueue == NULL){
    while(true){
      led.red();
      delay(500);
    }
  }
  if(systemConfig.wifiEnabled)
  {
    if(wifi.begin())
    {
        wifi.print();

        wifi.setSystemConfig(systemConfig);

        wifi.setOffsets(currentOffsets);
    }
  }

  lastTelemetryMillis = millis();

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
    taskData,
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
    static AttitudeData attitudeData;
    static NavigationData nav; 
    static ServoPositions angle;
    static ImuOffsets localOffsets; 
  //===================TO  ATTITUDE==============
    if(systemConfig.flightMode == FlightMode::CONFIG && newOffsetsAvailable){
      portENTER_CRITICAL(&offsetMux);

      localOffsets = currentOffsets;

      newOffsetsAvailable = false;

      portEXIT_CRITICAL(&offsetMux);
    }

    imu.update();

    pid.setAngles(angle, attitudeData, nav, useNewValues); 


    elevator.write(angle.elevator);
    leftAlieron.write(angle.leftAlieron);
    rightAlieron.write(angle.rightAlieron);

  //===================TO  ATTITUDE==============
  //=====================FOR SEND================
    attitudeData.pitch = imu.getPitch() - localOffsets.pitch;
    attitudeData.roll = imu.getRoll() - localOffsets.roll;
    attitudeData.yaw = imu.getYaw() - localOffsets.yaw;

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
    if(xQueueReceive(navigationQueue,&nav,0)){useNewValues = true;}
  //====================TO  RECEIVE==============
  
    vTaskDelayUntil(&lastWake,period);
  }
}
//=================================================================TASK AT CORE 0===========================================================================
//==========================================================================================================================================================
//=================================================================TASK AT CORE 1===========================================================================
void taskData(void *pv){

  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(100); //10HZ
  static bool isImuValid = false;

  while(true){

    static NavigationData navigationData;
    static AttitudeData attitude;
    
    bmp.update();

    if(gps.update()){led.green(); /*Serial.println("GPS Atualizado");*/} 
    else{led.blue();}

    isImuValid = readAttitudeData(attitude);

    telemetry.update(isImuValid);
      
    sendNavigationData(navigationData);

    handleOutputs();//Send LoRa and register in SD_Card

    if(systemConfig.flightMode == FlightMode::CONFIG){
      handleWifi(attitude, navigationData);
    }

    vTaskDelayUntil(&lastWake, period);
  }
}
//=================================================================TASK AT CORE 1===========================================================================
//==========================================================================================================================================================
//================================================================OTHERS FUNCTIONS==========================================================================
void handleWifi(const AttitudeData& attitude, const NavigationData& navigationData){
    if(!systemConfig.wifiEnabled)
        return;

    wifi.update();

    wifi.setAttitude(attitude);

    wifi.setNavigation(navigationData);

    wifi.setSystemStatus(systemStatus);

    switch(wifi.getPendingEvent())
    {
        case SystemEvent::NONE:
            return;

        case SystemEvent::OFFSET_CHANGED:

            portENTER_CRITICAL(&offsetMux);

            currentOffsets = wifi.getOffsets();

            newOffsetsAvailable = true;

            portEXIT_CRITICAL(&offsetMux);

            nvs.saveOffsets(currentOffsets);

            wifi.setOffsets(currentOffsets);

            break;

        case SystemEvent::SYSTEM_CHANGED:

            systemConfig = wifi.getSystemConfig();

            nvs.saveSystem(systemConfig);

            wifi.setSystemConfig(systemConfig);

            break;

        case SystemEvent::LORA_CHANGED:

            systemConfig = wifi.getSystemConfig();

            nvs.saveLora(systemConfig.lora);

            lora.reconfigure(systemConfig.lora);

            wifi.setSystemConfig(systemConfig);

            break;

        case SystemEvent::START_FLIGHT:

            systemConfig.flightMode =
                FlightMode::FLIGHT;

            wifi.setFlightMode(
                systemConfig.flightMode
            );

            break;

        case SystemEvent::RESTART:

            ESP.restart();

            return;
    }

    wifi.clearPendingEvent();
}
void handleOutputs(){

    bool powerMode = isLowPowerModeEnabled();
      //it runs when the BATTERY_LEVEL its HIGH
    if(!powerMode){

      if(millis() - lastTelemetryMillis > systemConfig.telemetryPeriodMs){ //The real time to loop this is 1E3 seconds? because of de vTaskDelay

        lastTelemetryMillis = millis();

        lora.send(telemetry.getLoraPacket(powerMode));
      
        if(!sd.saveLine(telemetry.getCsv())){Serial.println("Error ao gravar SD....");}

        Serial.println(telemetry.getJson());
      }
    }
    //it runs when the BATTERY_LEVEL its LOW
      else{        
        if(millis() - lastTelemetryMillis > systemConfig.lowBatteryTelemetryPeriodMs){ //1E4ms
          
          lastTelemetryMillis = millis();

          lora.send(telemetry.getLoraPacket(powerMode));
        }
      }
}
void sendNavigationData(NavigationData& navigationData){
    
  const bool gpsValid = gps.isValid();

    navigationData.latitude =
        gpsValid ? gps.getLatitude() : 0.0;

    navigationData.longitude =
        gpsValid ? gps.getLongitude() : 0.0;

    navigationData.gpsAltitude =
        gpsValid ? gps.getAltitude() : 0.0f;

    navigationData.course =
        gpsValid ? gps.getCourse() : 0.0f;

    navigationData.satellites =
        gpsValid ? gps.getSatellites() : 0;

    navigationData.baroAltitude =
        bmp.getRawAltitude();

    navigationData.temperature =
        bmp.getTemperature();

    navigationData.battery =
        ads.batteryLevel();

    xQueueOverwrite(
        navigationQueue,
        &navigationData
    );
}
bool readAttitudeData(AttitudeData& attitude){
    return xQueueReceive(
        attitudeQueue,
        &attitude,
        0
    ) == pdTRUE;
}
bool isLowPowerModeEnabled(){
    return ads.batteryLevel() < systemConfig.batteryLimit;
}