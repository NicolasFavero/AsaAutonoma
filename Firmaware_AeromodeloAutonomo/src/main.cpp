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
volatile bool newPidAvailable = false;

bool wifiEnabled = false;
bool wifiTemporary = false;

unsigned long wifiStartMillis = 0;
//==================================================================STRUCTS===============================================================================
ImuOffsets currentOffsets; 
PidConfig currentPidConfig;
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
Telemetry telemetry;

//Visual Debug
Led led;

//========================================================================================================================================================
//==============================================================FREEERTOS QUEUES===============================================================================
QueueHandle_t attitudeQueue;
QueueHandle_t navigationQueue;
portMUX_TYPE offsetMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE pidMux = portMUX_INITIALIZER_UNLOCKED;
//========================================================================================================================================================
//=================================================================PROTOTYPES=============================================================================
// TASKS
void taskControl(void *pv);
void taskData(void *pv);

// TASK CONTROL HELPERS
bool readAttitudeData(AttitudeData& attitude);

// TASK DATA HELPERS
void sendNavigationData(NavigationData& navigationData);
void handleWifiTimeout();
void handleOutputs(FlightMode mode);
void handleWifi(const AttitudeData& attitude, const NavigationData& navigationData);
void updateGpsStatus();
void setupWifi();
TelemetryData buildTelemetryData(AttitudeData& attitude);
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
  nvs.loadPid(currentPidConfig);

  pid.pitch.kp = currentPidConfig.pitch.kp;
  pid.pitch.ki = currentPidConfig.pitch.ki;
  pid.pitch.kd = currentPidConfig.pitch.kd;

  pid.roll.kp = currentPidConfig.roll.kp;
  pid.roll.ki = currentPidConfig.roll.ki;
  pid.roll.kd = currentPidConfig.roll.kd;

  pid.yaw.kp = currentPidConfig.yaw.kp;
  pid.yaw.ki = currentPidConfig.yaw.ki;
  pid.yaw.kd = currentPidConfig.yaw.kd;

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
  setupWifi();

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

    if(systemConfig.flightMode == FlightMode::CONFIG && newPidAvailable){
      portENTER_CRITICAL(&pidMux);

      pid.pitch.kp = currentPidConfig.pitch.kp;
      pid.pitch.ki = currentPidConfig.pitch.ki;
      pid.pitch.kd = currentPidConfig.pitch.kd;

      pid.roll.kp = currentPidConfig.roll.kp;
      pid.roll.ki = currentPidConfig.roll.ki;
      pid.roll.kd = currentPidConfig.roll.kd;

      pid.yaw.kp = currentPidConfig.yaw.kp;
      pid.yaw.ki = currentPidConfig.yaw.ki;
      pid.yaw.kd = currentPidConfig.yaw.kd;

      newPidAvailable = false;

      portEXIT_CRITICAL(&pidMux);
    }

    imu.update();

    pid.setAngles(angle, attitudeData, nav, useNewValues); 


    elevator.write(angle.elevator);
    leftAlieron.write(angle.leftAileron);
    rightAlieron.write(angle.rightAileron);

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

    attitudeData.isImuOk = imu.isHealthy();

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

    readAttitudeData(attitude);

    systemStatus.bmpOk = bmp.update();
    systemStatus.imuOk = attitude.isImuOk;

    if(gps.update()){led.green(); /*Serial.println("GPS Atualizado");*/} 
    else{led.blue();}

  

    telemetry.update(buildTelemetryData(attitude));

    sendNavigationData(navigationData);

    handleOutputs(systemConfig.flightMode);//Sends LoRa Packets and registers logs in SD_Card, it update systemStatus.loraOk

    if(systemConfig.flightMode != FlightMode::FLIGHT){


      if(wifiEnabled){
        handleWifi(attitude, navigationData);
        updateGpsStatus();
      }
      else{      
        handleWifiTimeout();
      }
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

        case SystemEvent::PID_CHANGED:

            portENTER_CRITICAL(&pidMux);

            currentPidConfig = wifi.getPidConfig();

            newPidAvailable = true;

            portEXIT_CRITICAL(&pidMux);

            nvs.savePid(currentPidConfig);

            wifi.setPidConfig(currentPidConfig);

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
                FlightMode::COUNTDOWN;

            wifi.setFlightMode(
                systemConfig.flightMode
            );

            break;

        case SystemEvent::END_FLIGHT:

            systemConfig.flightMode =
                FlightMode::LANDED;

            wifi.setFlightMode(
                systemConfig.flightMode
            );

            break;

        case SystemEvent::CHECK_SD:

            systemStatus.sdOk = sd.saveLine(
                telemetry.getCsvHeader()
            );

            break;

        case SystemEvent::DELETE_ALL_LOGS:

            if(sd.removeAllLogs())
            {
                Serial.println("Todos os logs removidos.");
            }
            else
            {
                Serial.println("Falha ao remover logs.");
            }

            break;
        case SystemEvent::DELETE_LOG:

            if(sd.removeFile(wifi.getRequestedLog()))
            {
                Serial.println("Log removido.");
            }
            else
            {
                Serial.println("Falha ao remover log.");
            }

            break;
        case SystemEvent::RENAME_LOG:

            if(sd.renameFile(wifi.getRequestedLog(), wifi.getRenameTarget()))
            {
                Serial.println("Log renomeado.");
            }
            else
            {
                Serial.println("Falha ao renomear log.");
            }

            break;
        case SystemEvent::TELEMETRY_REQUEST:

            wifi.setTelemetryJson(telemetry.buildLoraPacket(false));

            break;

        case SystemEvent::RESTART:

            ESP.restart();

            return;
    }

    wifi.clearPendingEvent();
}
void handleOutputs(FlightMode mode){
    bool lowPower = isLowPowerModeEnabled();

    uint32_t period;
    bool sendLora = true;
    bool saveSd = false;

    switch(mode)
    {
        case FlightMode::CONFIG:

            period = systemConfig.telemetryPeriodMs;

            sendLora = systemConfig.preFlightTelemetryEnabled; // futura função

            break;

        case FlightMode::COUNTDOWN:

            period = systemConfig.telemetryPeriodMs;

            sendLora = true;
            saveSd = true;
            // nos últimos 5 s você muda esse período
            // no código do countdown, não aqui.

            break;

        case FlightMode::FLIGHT:

            period = lowPower
                ? systemConfig.lowBatteryTelemetryPeriodMs
                : systemConfig.telemetryPeriodMs;

            sendLora = true;
            saveSd = !lowPower;

            break;

        case FlightMode::LANDED:

            period = systemConfig.lowBatteryTelemetryPeriodMs;

            sendLora = true;
            saveSd = false;

            break;
    }

    if(millis() - lastTelemetryMillis < period)
        return;

    lastTelemetryMillis = millis();

    if(sendLora)
    {
        systemStatus.loraOk =
            lora.send(telemetry.buildLoraPacket(lowPower));
    }

    if(saveSd)
    {
        systemStatus.sdOk =
            sd.saveLine(telemetry.buildCsv());
    }

    if(FlightMode::CONFIG == mode) {Serial.println(telemetry.buildLoraPacket(false));}
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
        gps.getSatellites();

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
void updateGpsStatus(){
    if(!gps.hasCommunication())
    {
        systemStatus.gpsStatus = GpsStatus::NO_COMMUNICATION;
        return;
    }

    if(!gps.isValid())
    {
        systemStatus.gpsStatus = GpsStatus::NO_FIX;
        return;
    }

    if(gps.getSatellites() < 6)
    {
        systemStatus.gpsStatus = GpsStatus::POOR_FIX;
        return;
    }

    systemStatus.gpsStatus = GpsStatus::GOOD_FIX;
}
void setupWifi(){
    // Nunca voou: liga normalmente
    /*if(systemConfig.flightMode == FlightMode::CONFIG)
    {
        wifiEnabled = true;
        wifiTemporary = false;
    }
    // Já voou: só liga se detectar 3,3 V no A3
    else if(ads.adcVoltage(3) > 3.0f)
    {
        wifiEnabled = true;
        wifiTemporary = true;
        wifiStartMillis = millis();
    }*/

    //if(!wifiEnabled)
        //return;

    if(wifi.begin())
    {
        wifiEnabled = true;

        wifi.attachSdLogger(&sd);

        wifi.print();

        wifi.setSystemConfig(systemConfig);

        wifi.setOffsets(currentOffsets);

        wifi.setPidConfig(currentPidConfig);
    }
    else
    {
        wifiEnabled = false;
    }
}
void handleWifiTimeout(){
    if(!wifiEnabled || !wifiTemporary)
        return;

    // Se alguém conectou, mantém o AP ligado
    if(wifi.hasClient())
    {
        wifiTemporary = false;
        return;
    }

    // Ninguém conectou durante 20 s
    if(millis() - wifiStartMillis >= 20000)
    {
        wifi.stop();

        wifiEnabled = false;

        wifiTemporary = false;
    }
}
TelemetryData buildTelemetryData(AttitudeData& attitude){

    TelemetryData data{};

    data.state = systemConfig.flightMode;

    data.gpsOk = gps.isValid();
    data.imuOk = attitude.isImuOk;

    if(attitude.isImuOk)
    {
        data.pitch = attitude.pitch;
        data.roll  = attitude.roll;
        data.yaw   = attitude.yaw;

        data.accX = attitude.accX;
        data.accY = attitude.accY;
        data.accZ = attitude.accZ;

        data.gyroX = attitude.gyroX;
        data.gyroY = attitude.gyroY;
        data.gyroZ = attitude.gyroZ;

        data.magX = attitude.magX;
        data.magY = attitude.magY;
        data.magZ = attitude.magZ;
    }

    if(data.gpsOk)
    {
        data.latitude   = gps.getLatitude();
        data.longitude  = gps.getLongitude();
        data.gpsAltitude = gps.getAltitude();
        data.course     = gps.getCourse();

        data.day    = gps.getDay();
        data.month  = gps.getMonth();
        data.year   = gps.getYear();

        data.hour   = gps.getHour();
        data.minute = gps.getMinute();
        data.second = gps.getSecond();
    }

    data.satellites = gps.getSatellites();

    data.baroAltitude = bmp.getRawAltitude();
    data.temperature  = bmp.getTemperature();

    data.battery = ads.batteryLevel();

    return data;
}