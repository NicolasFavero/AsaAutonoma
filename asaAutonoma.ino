#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include "esp_adc_cal.h"
#include "esp_task_wdt.h"
#include <SdFat.h>
#include <SPI.h>
#define SD_MISO 5
#define SD_MOSI 7
#define SD_SCK 6
#define SD_CS 9
//OBS: NOTE QUE O CÓDIGO FOI FEITO PARA O MÓDULO GY-91, O QUAL INTEGRA MPU9250 E BMP280
#define SDA 4
#define SCL 3
#define anguloNeutro 60
#define pitchDesejado 0
#define pinE 1  //servo esquerdo
#define pinD 2  //servo direito
#define pinBtn 10
#define xgzPin 8  //tubo de pitot
#define channelE 0
#define channelD 1
#define ADC_ATTEN ADC_ATTEN_DB_11
#define ADC_WIDTH ADC_WIDTH_BIT_12
esp_adc_cal_characteristics_t adc_chars;
Adafruit_BMP280 bmp;
MPU9250_asukiaaa mpu;
float mapF(float angulo, int in_min, int in_max, int out_min, int out_max) {
  return (angulo - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
class Files {
public:
  SdFat sd;
  SdFile file;
  char currentFile[20];
  bool fileOpen = false;

  const char *headersCsv =
    "tempo_ms,altitude,accX,accY,accZ,gyroX,gyroY,gyroZ,pitch,roll,angulo,angLeft,angRight";

  void begin() {
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI);

    if (!sd.begin(SD_CS, SD_SCK_MHZ(20))) {
      Serial.println("ERRO SD");
      while (1)
        ;
    }
  }

  void createFile() {
    if (fileOpen) return;

    getFileName();
    openFile();
    writeLine(headersCsv);

    Serial.print("LOG INICIADO: ");
    Serial.println(currentFile);
  }

  void saveLine(const char *line) {
    if (!fileOpen) return;

    file.println(line);

    static uint8_t cnt = 0;
    if (++cnt >= 5) {
      file.sync();
      cnt = 0;
    }
  }

  void closeFile() {
    if (!fileOpen) return;
    file.sync();
    file.close();
    fileOpen = false;
    Serial.println("LOG FINALIZADO");
  }

private:
  void openFile() {
    if (!file.open(currentFile, O_WRITE | O_CREAT | O_TRUNC)) {
      Serial.println("Erro abrir arquivo");
      while (1)
        ;
    }
    fileOpen = true;
  }

  void writeLine(const char *l) {
    file.println(l);
    file.sync();
  }

  void getFileName() {
    for (int i = 1; i < 10000; i++) {
      snprintf(currentFile, sizeof(currentFile), "/voo%d.csv", i);
      if (!sd.exists(currentFile)) return;
    }
    strcpy(currentFile, "/voo9999.csv");
  }
};

class Asa {
public:

  float kp = 40;
  float kd = 4;  //kd = kp * 0.1

  float vMin = 0.2;
  float vMax = 2.7;
  float pMax = 1000.0;

  float densidadeDoAr = 1.17;
  float graus = 30.0;

  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;

  float pitch = 0;
  float roll = 0;

  float vel;
  float pressure;

  float angulo;
  float offset;
  float offsetRight;
  float offsetLeft;
  float alt;
  float firstAlt;

  unsigned long lastTimeAtt = 0;

  void begin() {
    Wire.begin(SDA, SCL);
    initBmp280();
    mpu.beginAccel();
    mpu.beginGyro();
    initADCcal();
    pressure = getPressure();
    graus = bmp.readTemperature();
    densidadeDoAr = getDensidadeDoAr();
    lastTimeAtt = micros();
  }
  void initBmp280() {
    if (!bmp.begin(0x76)) {
      Serial.println(F("BMP280 não encontrado!"));
      while (1) delay(10);
    }
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X4,   // temperatura
      Adafruit_BMP280::SAMPLING_X16,  // pressão (mais importante)
      Adafruit_BMP280::FILTER_X16,    // filtro IIR
      Adafruit_BMP280::STANDBY_MS_1);
  }
  void atualizarValores() {

    mpu.accelUpdate();
    mpu.gyroUpdate();
    accY = mpu.accelY();
    accZ = mpu.accelZ() * -1.0;
    accX = mpu.accelX();
    gyroZ = mpu.gyroZ();
    gyroY = mpu.gyroY();
    gyroX = mpu.gyroX();

    //vel = getVel();

    calcularPitchRoll();
    //pressure = getPressure();
    angulo = getAngulo();
    createOffset();
    alt = bmp.readAltitude(1013.25);
  }
  void print() {
    Serial.print("Pitch: ");
    Serial.print(pitch, 2);
    Serial.print(" | Roll: ");
    Serial.print(roll, 2);

    Serial.print(" | Angulo: ");
    Serial.print(angulo, 2);
    Serial.print(" | OffSet: ");
    Serial.print(offset, 2);

    Serial.print(" | OffSetRight: ");
    Serial.print(offsetRight, 2);
    Serial.print(" | offSetLeft: ");
    Serial.print(offsetLeft, 2);

    Serial.print(" | GyroX: ");
    Serial.print(gyroX, 2);
    Serial.print(" | AccX: ");
    Serial.print(accX, 2);
    Serial.print(" | AccY: ");
    Serial.print(accY, 2);
    Serial.print(" | AccZ: ");
    Serial.println(accZ, 2);
  }
  void calcularPitchRoll() {
    unsigned long now = micros();
    float dt = (now - lastTimeAtt) * 1e-6;
    lastTimeAtt = now;

    if (dt <= 0 || dt > 0.1) return;

    float pitch_acc = atan2(accY, accZ) * 180.0 / PI;
    float roll_acc = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;

    float pitch_gyro = pitch + gyroY * dt;
    float roll_gyro = roll + gyroX * dt;

    const float alpha = 0.98;

    pitch = alpha * pitch_gyro + (1 - alpha) * pitch_acc;
    roll = alpha * roll_gyro + (1 - alpha) * roll_acc;
  }
  float getVel() {
    if (pressure < 5.0) return 0.0;
    float vel = sqrt(2 * pressure / densidadeDoAr);
    return vel;
  }
  float getAngulo() {  //aqui nao ta calculando densidade do ar pois estou sem o tubo de pitot ainda
    float erro = pitchDesejado - pitch;
    float desidadeDoAr = 1.225;
    float correction = kp * erro / 20 /* / (vel*vel + 1) */ - (0.02 * gyroX);
    float anguloServo = correction + anguloNeutro;
    return constrain(anguloServo, 0, 180);
  }
  void createOffset() {
    int max = 60;
    int min = max * -1;
    offset = mapF(roll, 20, -20, max, min);
    offset = constrain(offset, min, max);
    if (roll > 0) {
      offsetRight = offset * 1.0;
      offsetLeft = offset * 0.5;
    } else if (roll <= 0) {
      offsetLeft = offset * 0.5;
      offsetRight = offset * -1.0;
    }
  }

private:
  void initADCcal() {
    analogReadResolution(12);
    analogSetPinAttenuation(xgzPin, ADC_11db);

    esp_adc_cal_value_t tipo = esp_adc_cal_characterize(
      ADC_UNIT_1,
      ADC_ATTEN,
      ADC_WIDTH,
      1100,  // Vref padrão (será ignorado se tiver eFuse)
      &adc_chars);

    Serial.print("ADC calibrado usando: ");
    if (tipo == ESP_ADC_CAL_VAL_EFUSE_TP) {
      Serial.println("eFuse Two Point");
    } else if (tipo == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      Serial.println("eFuse Vref");
    } else {
      Serial.println("Vref DEFAULT (1100 mV)");
    }
  }
  float readADCvoltageAvg(int pin, int amostras = 15) {
    uint32_t soma = 0;

    for (int i = 0; i < amostras; i++) {
      soma += analogRead(pin);
    }

    uint32_t raw = soma / amostras;
    uint32_t mv = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
    return mv / 1000.0;
  }
  float getDensidadeDoAr() {
    float denAr = pressure / (287.0 * (graus + 273.15));
    return denAr;
  }
  float getPressure() {
    float adc = readADCvoltageAvg(xgzPin);
    float pressure = mapF(adc, vMin, vMax, -pMax, pMax);
    return pressure;
  }
};
class CreateCsv {
public:
  void make(char *out, size_t size, Asa &asa) {

    snprintf(out, size,
             "%lu,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%.2f,%.2f",
             millis(),
             (asa.alt - asa.firstAlt),
             asa.accX, asa.accY, asa.accZ,
             asa.gyroX, asa.gyroY, asa.gyroZ,
             asa.pitch, asa.roll,
             asa.angulo, (asa.angulo - asa.offsetLeft), (asa.angulo - asa.offsetRight));
  }
};
class Servo {
public:
  void begin() {
    ledcSetup(channelE, 50, 12);
    ledcAttachPin(pinE, channelE);
    ledcSetup(channelD, 50, 12);
    ledcAttachPin(pinD, channelD);
  }
  void write(float angulo, int channel, bool reverse = false) {
    angulo = constrain(angulo, 0, 180);
    float us;
    if (reverse) {
      us = mapF(angulo, 0, 180, 600, 2400);
    } else {
      us = mapF(angulo, 180, 0, 600, 2400);
    }
    uint32_t duty = (us * ((1 << 12) - 1)) / 20000;
    ledcWrite(channel, duty);
  }
  void imAlive() {

    write(160, channelD);
    write(160, channelE);
    delay(100);
    write(20, channelD);
    write(20, channelE);
    delay(100);
    write(90, channelD);
    write(90, channelE);
  }
};
Asa asa;
Servo servo;
CreateCsv csv;
Files sd;
String leituras = "";
void setup() {
  esp_task_wdt_init(5, true);
  esp_task_wdt_add(NULL);
  //----------------------------------------------------------------
  Serial.begin(115200);
  pinMode(pinBtn, INPUT_PULLUP);
  //Wire.begin() e SPI.begin() está dentro da Class Asa / Class Files
  //-----------------------------------------------------------------
  asa.begin();
  servo.begin();
  sd.begin();
  //-----------------------------------------------------------------
  servo.imAlive();
}
void loop() {
  esp_task_wdt_reset();



  if (digitalRead(pinBtn) == 0) {
    sd.createFile();
    servo.imAlive();

    Serial.println("botao apertadooo");  //debug, tava meio estranho e tá ajudando a saber quando entra
    asa.firstAlt = bmp.readAltitude(1013.25);
    unsigned long oldMillis = millis();



    while (millis() - oldMillis < 1000) {  //delay escroto, teste de protótipo por enquanto
      asa.atualizarValores();
      esp_task_wdt_reset();
    }



    oldMillis = millis();

    while (millis() - oldMillis < 3E4 && digitalRead(pinBtn) == 1) {  //ta tudo bagunçado, mas é para teste só, ajeitar dps

      uint32_t next = micros();

      if ((int32_t)(micros() - next) >= 0) {
        next += 5000;

        esp_task_wdt_reset();

        asa.atualizarValores();
        asa.print();

        float ang = asa.angulo;

        servo.write((ang - asa.offsetRight), channelD);  // args (angulo, pino, reverse = false)
        servo.write((ang - asa.offsetLeft), channelE, true);


        static unsigned long lastLog = 0;
        if (millis() - lastLog >= 50) {  // 20 Hz
          lastLog = millis();
          char payload[256];
          csv.make(payload, sizeof(payload), asa);
          sd.saveLine(payload);
        }
      }
    }
    sd.closeFile();
    delay(1000);
  }


  asa.atualizarValores();
  asa.print();
  yield();
}