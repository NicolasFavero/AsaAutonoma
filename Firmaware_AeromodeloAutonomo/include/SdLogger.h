#pragma once

#include <Arduino.h>
#include <SdFat.h>

class SdLogger {
public:
    explicit SdLogger(uint8_t csPin);

    bool begin();

    bool createFile();
    bool saveLine(const char* line);

    bool listFiles(String& json);
    bool removeFile(const char* filename);
    bool removeAllLogs();
    bool renameFile(const char* oldName, const char* newName);
    bool openRead(const char* filename, File32& file);

    void flush();
    void closeFile();

    bool isOpen() const;
    const char* getFileName() const;


private:
    SdFat sd;
    SdFile file;

    uint8_t csPin = 0;

    bool fileOpen = false;

    bool mounted = false;

    uint16_t syncCounter = 0;

    char currentFile[16] = "";

    static constexpr uint8_t SYNC_INTERVAL = 10;

    bool openFile();
    void generateFileName();
    bool reopenCurrentFile();
};