#include "RF96W.h"

void LoRa::setFlag() {packetReceived = true;}

LoRa::LoRa(
    uint8_t cs,
    uint8_t dio0,
    uint8_t rst,
    const LoraConfig& config
)
:
module(
    cs,
    dio0,
    rst,
    RADIOLIB_NC
),
radio(&module),
config(config)
{
    packetBuffer[0] = '\0';
}

bool LoRa::begin()
{
    uint8_t state = radio.begin(
        config.frequency,
        config.bandwidth,
        config.spreadingFactor,
        config.codingRate,
        config.syncWord,
        config.power,
        config.preambleLength
    );

    if(state != RADIOLIB_ERR_NONE)
        return false;

    radio.setDio0Action(
        setFlag,
        RISING
    );

    state = radio.startReceive();

    return state == RADIOLIB_ERR_NONE;
}
bool LoRa::send(const char* msg) {

    if (msg == nullptr) {
        return false;
    }

    radio.standby();

    int state = radio.transmit(msg);

    radio.startReceive();

    return state == RADIOLIB_ERR_NONE;
}
bool LoRa::available() {return packetReceived;}
bool LoRa::receive() {

    if (!packetReceived) {
        return false;
    }

    packetReceived = false;

    String received;

    int state = radio.readData(received);

    if (state != RADIOLIB_ERR_NONE) {

        radio.startReceive();
        return false;
    }

    strncpy(
        packetBuffer,
        received.c_str(),
        sizeof(packetBuffer)
    );

    packetBuffer[sizeof(packetBuffer) - 1] = '\0';

    rssi = radio.getRSSI();
    snr  = radio.getSNR();

    radio.startReceive();

    return true;
}
const char* LoRa::getPacket() const {return packetBuffer;}
float LoRa::getRSSI() const {return rssi;}
float LoRa::getSNR() const {return snr;}
void LoRa::print(){
    Serial.print("PACOTE: ");
    Serial.println(getPacket());

    Serial.print("RSSI: ");
    Serial.println(getRSSI());

    Serial.print("SNR: ");
    Serial.println(getSNR());
}
void LoRa::setConfig(
    const LoraConfig& newConfig)
{
    config = newConfig;
}
const LoraConfig&
LoRa::getConfig() const
{
    return config;
}
bool LoRa::reconfigure(
    const LoraConfig& newConfig)
{
    radio.standby();

    config = newConfig;

    int state = radio.begin(
        config.frequency,
        config.bandwidth,
        config.spreadingFactor,
        config.codingRate,
        config.syncWord,
        config.power,
        config.preambleLength
    );

    if(state != RADIOLIB_ERR_NONE)
        return false;

    radio.setDio0Action(
        setFlag,
        RISING
    );

    state = radio.startReceive();

    return state == RADIOLIB_ERR_NONE;
}