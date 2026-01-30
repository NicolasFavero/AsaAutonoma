# Sistema de Controle e Telemetria para Asa Autônoma (Protótipo)

Este projeto é um **protótipo em desenvolvimento** de um sistema embarcado para **asa autônoma / aeromodelo planador**, baseado em **ESP32-C3**.

O sistema faz a leitura de sensores, controla os servos das superfícies de comando e **registra os dados de voo em um cartão SD** para análise posterior.

---

## Funcionalidades

- Leitura de IMU (aceleração, giroscópio e magnetômetro)
- Medição de altitude e pressão atmosférica (BMP280)
- Leitura de velocidade do ar via tubo de Pitot
- Controle de superfícies:
  - Leme
  - Aileron
  - Profundor
- Controle de servos
- Registro de dados de voo em cartão SD
- Estrutura preparada para expansão

---

## Status do projeto

**Protótipo / Em desenvolvimento**

O código ainda está em fase experimental e pode sofrer alterações.

---

## Tecnologias utilizadas

- ESP32-C3
- C / C++
- MPU9250
- BMP280
- Tubo de Pitot
- Servos
- Cartão SD

---

## Objetivo

Servir como base para testes e desenvolvimento de um sistema de controle para aeronaves experimentais.

---

## Dependências

### by Adafruit
- Adafruit BMP280 Library — v3.0.0

### by Arduino
- SPI — incluído no core
- Wire — incluído no core

### by Espressif Systems
- esp_adc_cal — incluído no core
- esp_task_wdt — incluído no core

### by asukiaaa
- MPU9250_asukiaaa — v1.5.13

### Outros
- SdFat — v2.3.0

---

## Ambiente de desenvolvimento

- Arduino IDE 2.3.7
- Arduino Core for ESP32 — v2.x
- Placa alvo: ESP32-C3

---

## Observações

Projeto voltado para fins experimentais.
