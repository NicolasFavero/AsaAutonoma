#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "DataTypes.h"


constexpr char AP_SSID[] = "AsaAutonoma";

constexpr char AP_PASSWORD[] = "12345678";

constexpr uint8_t MAX_CLIENTS = 1;




class WifiAP
{
public:

    WifiAP();

    /*
    ==========================================================
                        AP Control
    ==========================================================
    */

    bool begin();

    void print();
    
    void stop();

    void update();

    bool isRunning() const;

    bool hasClient() const;

    uint8_t getConnectedClients() const;

    /*
    ==========================================================
                        Incoming Data
    ==========================================================
    */

    void setAttitude(const AttitudeData& data);

    void setNavigation(const NavigationData& data);

    void setSystemStatus(const SystemStatus& data);

    /*
    ==========================================================
                    Current Configuration
    ==========================================================
    */

    void setSystemConfig(const SystemConfig& config);

    void setOffsets(const ImuOffsets& offsets);

    void setFlightMode(FlightMode mode);

    FlightMode getFlightMode() const;
    /*
    ==========================================================
                        Event System
    ==========================================================
    */

    SystemEvent getPendingEvent() const;

    void clearPendingEvent();

    const SystemConfig& getSystemConfig() const;

    const ImuOffsets& getOffsets() const;

    /*
    ==========================================================
                           States
    ==========================================================
    */

private:

    /*
    ==========================================================
                        WiFi
    ==========================================================
    */

    WebServer server{80};

    bool running = false;

    /*
    ==========================================================
                        Cached Data
    ==========================================================
    */

    AttitudeData attitude;

    NavigationData navigation;

    SystemStatus status;

    SystemConfig systemConfig;

    ImuOffsets offsets;

    /*
    ==========================================================
                        Events
    ==========================================================
    */

    volatile SystemEvent pendingEvent =
        SystemEvent::NONE;

    /*
    ==========================================================
                        Routes
    ==========================================================
    */

    void configureRoutes();

    /*
    ==========================================================
                        Pages
    ==========================================================
    */

    void buildHomePage();
    char* homePage = nullptr;
    void handleHome(); //não existe mais, era inutil, mas deixei só pra ficar organizado, para implementação futura

    void handleConfig();

    void handleStatus();

    void handleOffsets();

    void handleSystem();

    void handleLora();

    void handleStartFlight();

    void handleRestart();

    void handleNotFound();

    /*
    ==========================================================
                        Helpers
    ==========================================================
    */

    void sendJsonStatus();

    void sendJsonConfig();

    void sendJsonNavigation();

    void sendJsonAttitude();

    void sendJsonSuccess(
        const char* message
    );

    void sendJsonError(
        const char* message
    );

    static constexpr size_t STATUS_JSON_SIZE = 768;

    static constexpr size_t MESSAGE_JSON_SIZE = 128;

    char statusJson[STATUS_JSON_SIZE];

    char messageJson[MESSAGE_JSON_SIZE];
    /*
    ==========================================================
                        States
    ==========================================================
    */
    bool flightStarted = false;

    bool countdownRunning = false;
};