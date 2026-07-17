#include "WifiAP.h"
#include "Web/Web.h"


WifiAP::WifiAP(): server(80){}
bool WifiAP::begin(){
    if(running)
        return true;

    WiFi.mode(WIFI_AP);

    if(!WiFi.softAP(
        AP_SSID,
        AP_PASSWORD,
        1,
        false,
        MAX_CLIENTS))
    {
        return false;
    }

    buildHomePage();

    configureRoutes();

    server.begin();

    running = true;

    return true;
}
void WifiAP::buildHomePage(){
    const size_t len =
        snprintf(
            nullptr,
            0,
            Web::Pages::HOME_PAGE,
            Web::Style::CSS,
            Web::Components::MENU_COMPONENT,
            Web::Script::JS
        );

    if(homePage)
    {
        free(homePage);
        homePage = nullptr;
    }

    homePage =
        (char*)malloc(len + 1);

    //homePage = (char*)ps_malloc(len + 1);

    if(!homePage)
        return;

    snprintf(
        homePage,
        len + 1,
        Web::Pages::HOME_PAGE,
        Web::Style::CSS,
        Web::Components::MENU_COMPONENT,
        Web::Script::JS
    );
}
void WifiAP::stop(){
    if(!running)
        return;

    server.stop();

    WiFi.softAPdisconnect(true);

    WiFi.mode(WIFI_OFF);

    if(homePage)
    {
        free(homePage);

        homePage = nullptr;
    }

    running = false;
}
void WifiAP::print(){
    Serial.println();
    Serial.println("========== WiFi AP ==========");

    Serial.print("SSID: ");
    Serial.println(AP_SSID);

    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    Serial.print("Channel: ");
    Serial.println(WiFi.channel());

    Serial.print("Max Clients: ");
    Serial.println(MAX_CLIENTS);

    Serial.print("Connected: ");
    Serial.println(WiFi.softAPgetStationNum());

    Serial.println("=============================");
}
void WifiAP::update(){
    if(!running)
        return;

    server.handleClient();
}
bool WifiAP::isRunning() const{
    return running;
}
bool WifiAP::hasClient() const{
    return WiFi.softAPgetStationNum() > 0;
}
uint8_t WifiAP::getConnectedClients() const{
    return WiFi.softAPgetStationNum();
}
void WifiAP::setAttitude(const AttitudeData& data){
    attitude = data;
}
void WifiAP::setNavigation(const NavigationData& data){
    navigation = data;
}
void WifiAP::setSystemStatus(const SystemStatus& data){
    status = data;
}
void WifiAP::setSystemConfig(const SystemConfig& config){
    systemConfig = config;
}
void WifiAP::setOffsets(const ImuOffsets& newOffsets){
    offsets = newOffsets;
}
SystemEvent WifiAP::getPendingEvent() const{
    return pendingEvent;
}
void WifiAP::clearPendingEvent(){
    pendingEvent = SystemEvent::NONE;
}
const SystemConfig&
WifiAP::getSystemConfig() const{
    return systemConfig;
}
const ImuOffsets&
WifiAP::getOffsets() const{
    return offsets;
}
void WifiAP::configureRoutes(){
    /*
    ==========================================================
                        HOME
    ==========================================================
    */

    server.on("/", HTTP_GET, [this]()
    {
        handleHome();
    });

    /*
    ==========================================================
                        HTML PAGES
    ==========================================================
    */

    server.on("/page/status", HTTP_GET, [this]()
    {
        server.send_P(
            200,
            "text/html",
            Web::Pages::STATUS_PAGE
        );
    });

    server.on("/page/offsets", HTTP_GET, [this]()
    {
        server.send_P(
            200,
            "text/html",
            Web::Pages::OFFSET_PAGE
        );
    });

    server.on("/page/system", HTTP_GET, [this]()
    {
        server.send_P(
            200,
            "text/html",
            Web::Pages::SYSTEM_PAGE
        );
    });

    server.on("/page/lora", HTTP_GET, [this]()
    {
        server.send_P(
            200,
            "text/html",
            Web::Pages::LORA_PAGE
        );
    });

    server.on("/page/flight", HTTP_GET, [this]()
    {
        server.send_P(
            200,
            "text/html",
            Web::Pages::FLIGHT_PAGE
        );
    });

    server.on("/page/diagnostics", HTTP_GET, [this]()
    {
        server.send_P(
            200,
            "text/html",
            Web::Pages::DIAGNOSTIC_PAGE
        );
    });

    /*
    ==========================================================
                        API
    ==========================================================
    */

    server.on("/api/status", HTTP_GET, [this]()
    {
        handleStatus();
    });

    server.on("/api/config", HTTP_GET, [this]()
    {
        handleConfig();
    });

    server.on("/api/offsets", HTTP_POST, [this]()
    {
        handleOffsets();
    });

    server.on("/api/system", HTTP_POST, [this]()
    {
        handleSystem();
    });

    server.on("/api/lora", HTTP_POST, [this]()
    {
        handleLora();
    });

    server.on("/api/start", HTTP_POST, [this]()
    {
        handleStartFlight();
    });

    server.on("/api/restart", HTTP_POST, [this]()
    {
        handleRestart();
    });

    /*
    ==========================================================
                        404
    ==========================================================
    */

    server.onNotFound([this]()
    {
        handleNotFound();
    });
}
void WifiAP::handleHome(){
    if(!homePage)
    {
        server.send(
            500,
            "text/plain",
            "Home Page Error"
        );

        return;
    }

    server.send(
        200,
        "text/html",
        homePage
    );
}
void WifiAP::handleConfig(){
    sendJsonConfig();
}
void WifiAP::handleStatus(){
    sendJsonStatus();
}
void WifiAP::handleOffsets(){
    if(systemConfig.flightMode != FlightMode::CONFIG)
    {
        sendJsonError(
            "Configuration Locked"
        );
        return;
    }

    if(server.hasArg("pitch"))
        offsets.pitch =
            server.arg("pitch").toFloat();

    if(server.hasArg("roll"))
        offsets.roll =
            server.arg("roll").toFloat();

    if(server.hasArg("yaw"))
        offsets.yaw =
            server.arg("yaw").toFloat();

    pendingEvent =
        SystemEvent::OFFSET_CHANGED;

    sendJsonSuccess(
        "Offsets Updated"
    );
}
void WifiAP::handleSystem(){
    if(systemConfig.flightMode != FlightMode::CONFIG)
    {
        sendJsonError(
            "Configuration Locked"
        );
        return;
    }

    if(server.hasArg("battery"))
        systemConfig.batteryLimit =
            server.arg("battery").toFloat();

    if(server.hasArg("telemetry"))
        systemConfig.telemetryPeriodMs =
            server.arg("telemetry").toInt();

    if(server.hasArg("flightTime"))
        systemConfig.estimatedFlightTimeMin =
            server.arg("flightTime").toInt();

    if(server.hasArg("web"))
        systemConfig.telemetryWebEnabled =
            server.arg("web") == "1";

    if(server.hasArg("wifi"))
        systemConfig.wifiEnabled =
            server.arg("wifi") == "1";

    pendingEvent = SystemEvent::SYSTEM_CHANGED;

    sendJsonSuccess("System Updated");
}
void WifiAP::handleLora(){
    if(systemConfig.flightMode != FlightMode::CONFIG){
        sendJsonError(
            "Configuration Locked"
        );
        return;
    }

    if(server.hasArg("freq"))
        systemConfig.lora.frequency =
            server.arg("freq").toFloat();

    if(server.hasArg("bw"))
        systemConfig.lora.bandwidth =
            server.arg("bw").toFloat();

    if(server.hasArg("sf"))
        systemConfig.lora.spreadingFactor =
            server.arg("sf").toInt();

    if(server.hasArg("cr"))
        systemConfig.lora.codingRate =
            server.arg("cr").toInt();

    if(server.hasArg("power"))
        systemConfig.lora.power =
            server.arg("power").toInt();

    if(server.hasArg("sync"))
    {
        systemConfig.lora.syncWord =
            strtoul(
                server.arg("sync").c_str(),
                nullptr,
                16
            );
    }

    if(server.hasArg("pre"))
        systemConfig.lora.preambleLength =
            server.arg("pre").toInt();

    pendingEvent = SystemEvent::LORA_CHANGED;

    sendJsonSuccess("LoRa Updated");
}
void WifiAP::handleStartFlight()
{
    if(systemConfig.flightMode != FlightMode::CONFIG)
    {
        sendJsonError(
            "Flight Already Started"
        );
        return;
    }

    pendingEvent =
        SystemEvent::START_FLIGHT;

    sendJsonSuccess(
        "Countdown Started"
    );
}
void WifiAP::handleRestart(){
    pendingEvent =
        SystemEvent::RESTART;

    sendJsonSuccess(
        "Restart Requested"
    );
}
void WifiAP::handleNotFound(){
    server.send(
        404,
        "text/plain",
        "404"
    );
}
void WifiAP::sendJsonStatus(){
    
    const char* mode;

    switch(systemConfig.flightMode)
    {
        case FlightMode::CONFIG:
            mode = "CONFIG";
            break;

        case FlightMode::COUNTDOWN:
            mode = "COUNTDOWN";
            break;

        case FlightMode::FLIGHT:
            mode = "FLIGHT";
            break;

        case FlightMode::LANDED:
            mode = "LANDED";
            break;

        default:
            mode = "UNKNOWN";
            break;
    }

    snprintf(
        statusJson,
        sizeof(statusJson),

        "{"

        "\"flightMode\":\"%s\","

        "\"pitch\":%.2f,"
        "\"roll\":%.2f,"
        "\"yaw\":%.2f,"

        "\"pitchOffset\":%.2f,"
        "\"rollOffset\":%.2f,"
        "\"yawOffset\":%.2f,"

        "\"lat\":%.7f,"
        "\"lon\":%.7f,"

        "\"gpsAlt\":%.2f,"
        "\"baroAlt\":%.2f,"

        "\"battery\":%.2f,"

        "\"wifiEnabled\":%s,"
        "\"telemetryWeb\":%s,"

        "\"gpsSat\":%u,"

        "\"imuOk\":%s,"
        "\"gpsOk\":%s,"
        "\"bmpOk\":%s,"

        "\"telemetryMs\":%u,"

        "\"batteryLimit\":%.2f,"

        "\"wifiClients\":%u"

        "}",

        mode,

        attitude.pitch,
        attitude.roll,
        attitude.yaw,

        offsets.pitch,
        offsets.roll,
        offsets.yaw,

        navigation.latitude,
        navigation.longitude,

        navigation.gpsAltitude,
        navigation.baroAltitude,

        navigation.battery,

        // wifiEnabled
        systemConfig.wifiEnabled ? "true" : "false",

        // telemetryWeb
        systemConfig.telemetryWebEnabled ? "true" : "false",

        // gpsSat
        navigation.satellites,

        // imuOk
        status.imuOk ? "true" : "false",

        // gpsOk
        status.gpsOk ? "true" : "false",

        // bmpOk
        status.bmpOk ? "true" : "false",

        // telemetryMs
        systemConfig.telemetryPeriodMs,

        // batteryLimit
        systemConfig.batteryLimit,

        // wifiClients
        WiFi.softAPgetStationNum()
    );

    server.send(
        200,
        "application/json",
        statusJson
    );
}
void WifiAP::sendJsonSuccess(const char* message){

    snprintf(
        messageJson,
        sizeof(messageJson),

        "{\"success\":true,\"message\":\"%s\"}",

        message
    );

    server.send(
        200,
        "application/json",
        messageJson
    );
}
void WifiAP::sendJsonError(const char* message){

    snprintf(
        messageJson,
        sizeof(messageJson),

        "{\"success\":false,\"message\":\"%s\"}",

        message
    );

    server.send(
        400,
        "application/json",
        messageJson
    );
}
void WifiAP::sendJsonConfig(){
    snprintf(
        statusJson,
        sizeof(statusJson),

        "{"

        "\"pitchOffset\":%.2f,"
        "\"rollOffset\":%.2f,"
        "\"yawOffset\":%.2f,"

        "\"batteryLimit\":%.2f,"
        "\"telemetryMs\":%u,"
        "\"flightTime\":%u,"

        "\"wifiEnabled\":%s,"
        "\"telemetryWeb\":%s,"

        "\"frequency\":%.3f,"
        "\"bandwidth\":%.1f,"
        "\"spreadingFactor\":%u,"
        "\"codingRate\":%u,"
        "\"power\":%u,"
        "\"syncWord\":%u,"
        "\"preambleLength\":%u"

        "}",

        offsets.pitch,
        offsets.roll,
        offsets.yaw,

        systemConfig.batteryLimit,
        systemConfig.telemetryPeriodMs,
        systemConfig.estimatedFlightTimeMin,

        systemConfig.wifiEnabled ? "true" : "false",
        systemConfig.telemetryWebEnabled ? "true" : "false",

        systemConfig.lora.frequency,
        systemConfig.lora.bandwidth,
        systemConfig.lora.spreadingFactor,
        systemConfig.lora.codingRate,
        systemConfig.lora.power,
        systemConfig.lora.syncWord,
        systemConfig.lora.preambleLength
    );

    server.send(
        200,
        "application/json",
        statusJson
    );
}
void WifiAP::setFlightMode(FlightMode mode){
    systemConfig.flightMode = mode;
}
FlightMode WifiAP::getFlightMode() const{
    return systemConfig.flightMode;
}