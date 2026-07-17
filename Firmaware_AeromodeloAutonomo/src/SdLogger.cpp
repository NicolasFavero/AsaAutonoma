#include "SdLogger.h"

SdLogger::SdLogger(uint8_t csPin)
    : csPin(csPin) {
}

bool SdLogger::begin() {

    unsigned long start = millis();

    while (millis() - start < 300) {

        if (sd.begin(csPin, SD_SCK_MHZ(20))) {
            return true;
        }

        delay(50);
    }

    return false;
}

bool SdLogger::createFile() {

    if (fileOpen) {
        return true;
    }

    generateFileName();

    if (!openFile()) {
        return false;
    }

    Serial.print("LOG INICIADO: ");
    Serial.println(currentFile);

    return true;
}

bool SdLogger::openFile() {

    if (!file.open(
            currentFile,
            O_WRITE | O_CREAT | O_TRUNC)) {
        return false;
    }

    fileOpen = true;
    return true;
}

bool SdLogger::reopenCurrentFile() {

    if (fileOpen) {
        file.close();
    }

    fileOpen = false;

    if (!sd.begin(csPin, SD_SCK_MHZ(20))) {
        return false;
    }

    if (!file.open(
            currentFile,
            O_WRITE | O_APPEND)) {
        return false;
    }

    fileOpen = true;
    return true;
}

bool SdLogger::saveLine(const char* line) {

    if (!fileOpen) {
        return false;
    }

    if (!file.println(line)) {

        if (!reopenCurrentFile()) {
            return false;
        }

        if (!file.println(line)) {
            return false;
        }
    }

    syncCounter++;

    if (syncCounter >= SYNC_INTERVAL) {

        if (!file.sync()) {

            if (!reopenCurrentFile()) {
                return false;
            }
        }

        syncCounter = 0;
    }

    return true;
}

void SdLogger::closeFile() {
    if (!fileOpen) {
        return;
    }

    file.sync();
    file.close();

    syncCounter = 0;
    fileOpen = false;

    Serial.println("LOG FINALIZADO");
}

bool SdLogger::isOpen() const {
    return fileOpen;
}

const char* SdLogger::getFileName() const {
    return currentFile;
}

void SdLogger::generateFileName() {

    for (uint16_t i = 1; i < 10000; i++) {

        snprintf(
            currentFile,
            sizeof(currentFile),
            "/voo%u.csv",
            i
        );

        if (!sd.exists(currentFile)) {
            return;
        }
    }

    strcpy(currentFile, "/voo9999.csv");
}

void SdLogger::flush() {

    if (!fileOpen) {
        return;
    }

    file.sync();
}