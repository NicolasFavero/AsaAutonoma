<<<<<<< Updated upstream
# Sistema de Controle e Telemetria para Asa Autônoma (Protótipo)
=======
# Sistema de Controle e Telemetria para Aeromodelo Experimental (v1.0)
>>>>>>> Stashed changes

Este projeto é um **protótipo em desenvolvimento** de um sistema embarcado para **asa autônoma / aeromodelo planador**, baseado em **ESP32-C3**.

<<<<<<< Updated upstream
O sistema faz a leitura de sensores, controla os servos das superfícies de comando e **registra os dados de voo em um cartão SD** para análise posterior.
=======
Este projeto corresponde à **primeira versão (v1.0)** de um sistema embarcado desenvolvido para aquisição de dados e controle de um aeromodelo experimental.

O sistema é baseado em um **ESP32-C3** e integra sensores inerciais e ambientais através do módulo **GY-91 (MPU9250 + BMP280)**, além do armazenamento dos dados de voo em um cartão microSD.

O objetivo desta versão foi validar o hardware, desenvolver a arquitetura inicial do firmware e integrar os principais componentes eletrônicos utilizados no sistema.

---

## Objetivo

Desenvolver uma plataforma embarcada capaz de:

- Adquirir dados de sensores inerciais;
- Medir pressão atmosférica e altitude;
- Estimar a atitude da aeronave;
- Controlar os servomotores das superfícies móveis;
- Registrar os dados de voo em cartão microSD para análise posterior.
>>>>>>> Stashed changes

---

## Funcionalidades

<<<<<<< Updated upstream
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
=======
- Leitura do acelerômetro;
- Leitura do giroscópio;
- Leitura do magnetômetro;
- Estimativa de Pitch e Roll utilizando filtro complementar;
- Medição de pressão atmosférica;
- Cálculo de altitude utilizando BMP280;
- Controle dos servomotores;
- Registro automático dos dados em arquivos CSV no cartão microSD.
>>>>>>> Stashed changes

---

## Status do projeto

<<<<<<< Updated upstream
**Protótipo / Em desenvolvimento**

O código ainda está em fase experimental e pode sofrer alterações.
=======
✅ **Versão 1.0 concluída**

Esta versão representa a primeira implementação funcional do sistema e serviu como base para o desenvolvimento das versões posteriores do projeto.
>>>>>>> Stashed changes

---

## Tecnologias utilizadas

- ESP32-C3
- C / C++
- MPU9250
- BMP280
- Tubo de Pitot
- Servos
- Cartão SD

<<<<<<< Updated upstream
---
=======
- ESP32-C3;
- GY-91 (MPU9250 + BMP280);
- Módulo para cartão microSD;
- Servomotores.
>>>>>>> Stashed changes

## Objetivo

<<<<<<< Updated upstream
Servir como base para testes e desenvolvimento de um sistema de controle para aeronaves experimentais.
=======
- C / C++;
- Arduino IDE;
- Arduino Framework.
>>>>>>> Stashed changes

---

## Dependências

<<<<<<< Updated upstream
### Adafruit
- Adafruit BMP280 Library — v3.0.0

### Arduino
- SPI — incluído no core
- Wire — incluído no core

### Espressif Systems
- esp_adc_cal — incluído no core
- esp_task_wdt — incluído no core

### asukiaaa
- MPU9250_asukiaaa — v1.5.13

### Outros
- SdFat — v2.3.0
=======
- Adafruit BMP280 Library;
- MPU9250_asukiaaa;
- SdFat;
- SPI;
- Wire.
>>>>>>> Stashed changes

---

## Ambiente de desenvolvimento

<<<<<<< Updated upstream
- Arduino IDE 2.3.7
- Arduino Core for ESP32 — v2.x
- Placa alvo: ESP32-C3
=======
- IDE: Arduino IDE;
- Framework: Arduino;
- Placa alvo: ESP32-C3.
>>>>>>> Stashed changes

---

## Observações

<<<<<<< Updated upstream
Projeto voltado para fins experimentais.
=======
```text
Firmware/
└── Firmware.ino
```

---

## Aplicação

Projeto desenvolvido para fins educacionais e experimentais, integrando conhecimentos de eletrônica, programação embarcada, sensores e sistemas aeronáuticos. Esta versão serviu como base para o desenvolvimento das futuras versões do sistema de controle e telemetria.
>>>>>>> Stashed changes
