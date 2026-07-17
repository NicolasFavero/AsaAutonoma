#include "PreferencesManager.h"

namespace PreferencesKeys
{
    constexpr char NAMESPACE[] = "aircraft";

    /*
    ==========================================================
                            Offsets
    ==========================================================
    */

    constexpr char PITCH[] = "pitch";
    constexpr char ROLL[]  = "roll";
    constexpr char YAW[]   = "yaw";

    /*
    ==========================================================
                            System
    ==========================================================
    */

    constexpr char BATTERY_LIMIT[] = "battery";

    constexpr char TELEMETRY_MS[] = "telemetry";

    constexpr char WIFI_ENABLED[] = "wifi";

    constexpr char FLIGHT_TIME[] = "flight";

    /*
    ==========================================================
                            LoRa
    ==========================================================
    */

    constexpr char LORA_FREQ[] = "freq";

    constexpr char LORA_BW[] = "bw";

    constexpr char LORA_SF[] = "sf";

    constexpr char LORA_CR[] = "cr";

    constexpr char LORA_POWER[] = "power";

    constexpr char LORA_SYNC[] = "sync";

    constexpr char LORA_PRE[] = "pre";
}

/*
==========================================================
                    Private
==========================================================
*/

bool PreferencesManager::openNamespace(
    Preferences& nvs,
    bool readOnly)
{
    return nvs.begin(
        PreferencesKeys::NAMESPACE,
        readOnly
    );
}

/*
==========================================================
                Initialization
==========================================================
*/

bool PreferencesManager::initialize()
{
    Preferences nvs;

    if(!openNamespace(nvs, false)){
        return false;
    }
    bool initialized =
        nvs.isKey(
            PreferencesKeys::BATTERY_LIMIT
        );

    nvs.end();

    if(initialized){
        return true;
    }
    return restoreDefaults();
}

/*
==========================================================
                    Load
==========================================================
*/

bool PreferencesManager::loadOffsets(
    ImuOffsets& offsets)
{
    Preferences nvs;

    if(!openNamespace(nvs, true))
        return false;

    offsets.pitch =
        nvs.getFloat(
            PreferencesKeys::PITCH,
            0.0f
        );

    offsets.roll =
        nvs.getFloat(
            PreferencesKeys::ROLL,
            0.0f
        );

    offsets.yaw =
        nvs.getFloat(
            PreferencesKeys::YAW,
            0.0f
        );

    nvs.end();

    return true;
}

bool PreferencesManager::loadSystem(
    SystemConfig& config)
{
    Preferences nvs;

    if(!openNamespace(nvs, true))
        return false;

    config.batteryLimit =
        nvs.getFloat(
            PreferencesKeys::BATTERY_LIMIT,
            7.5f
        );

    config.telemetryPeriodMs =
        nvs.getUShort(
            PreferencesKeys::TELEMETRY_MS,
            100
        );

    config.wifiEnabled =
        nvs.getBool(
            PreferencesKeys::WIFI_ENABLED,
            true
        );

    config.estimatedFlightTimeMin =
        nvs.getUShort(
            PreferencesKeys::FLIGHT_TIME,
            30
        );

    nvs.end();

    return true;
}

bool PreferencesManager::loadLora(
    LoraConfig& config)
{
    Preferences nvs;

    if(!openNamespace(nvs, true))
        return false;

    config.frequency =
        nvs.getFloat(
            PreferencesKeys::LORA_FREQ,
            915.0f
        );

    config.bandwidth =
        nvs.getFloat(
            PreferencesKeys::LORA_BW,
            125.0f
        );

    config.spreadingFactor =
        nvs.getUChar(
            PreferencesKeys::LORA_SF,
            10
        );

    config.codingRate =
        nvs.getUChar(
            PreferencesKeys::LORA_CR,
            5
        );

    config.power =
        nvs.getUChar(
            PreferencesKeys::LORA_POWER,
            20
        );

    config.syncWord =
        nvs.getUChar(
            PreferencesKeys::LORA_SYNC,
            0x12
        );

    config.preambleLength =
        nvs.getUShort(
            PreferencesKeys::LORA_PRE,
            8
        );

    nvs.end();

    return true;
}

bool PreferencesManager::loadAll(
    SystemConfig& system,
    ImuOffsets& offsets)
{
    bool ok = true;

    ok &= loadSystem(system);

    ok &= loadOffsets(offsets);

    ok &= loadLora(system.lora);

    return ok;
}
/*
==========================================================
                    Save
==========================================================
*/

bool PreferencesManager::saveOffsets(
    const ImuOffsets& offsets)
{
    Preferences nvs;

    if(!openNamespace(nvs, false))
        return false;

    bool ok = true;

    ok &= (
        nvs.putFloat(
            PreferencesKeys::PITCH,
            offsets.pitch
        ) > 0
    );

    ok &= (
        nvs.putFloat(
            PreferencesKeys::ROLL,
            offsets.roll
        ) > 0
    );

    ok &= (
        nvs.putFloat(
            PreferencesKeys::YAW,
            offsets.yaw
        ) > 0
    );

    nvs.end();

    return ok;
}

bool PreferencesManager::saveSystem(
    const SystemConfig& config)
{
    Preferences nvs;

    if(!openNamespace(nvs, false))
        return false;

    bool ok = true;

    ok &= (
        nvs.putFloat(
            PreferencesKeys::BATTERY_LIMIT,
            config.batteryLimit
        ) > 0
    );

    ok &= (
        nvs.putUShort(
            PreferencesKeys::TELEMETRY_MS,
            config.telemetryPeriodMs
        ) > 0
    );

    ok &= (
        nvs.putBool(
            PreferencesKeys::WIFI_ENABLED,
            config.wifiEnabled
        ) > 0
    );

    ok &= (
        nvs.putUShort(
            PreferencesKeys::FLIGHT_TIME,
            config.estimatedFlightTimeMin
        ) > 0
    );

    nvs.end();

    return ok;
}

bool PreferencesManager::saveLora(
    const LoraConfig& config)
{
    Preferences nvs;

    if(!openNamespace(nvs, false))
        return false;

    bool ok = true;

    ok &= (
        nvs.putFloat(
            PreferencesKeys::LORA_FREQ,
            config.frequency
        ) > 0
    );

    ok &= (
        nvs.putFloat(
            PreferencesKeys::LORA_BW,
            config.bandwidth
        ) > 0
    );

    ok &= (
        nvs.putUChar(
            PreferencesKeys::LORA_SF,
            config.spreadingFactor
        ) > 0
    );

    ok &= (
        nvs.putUChar(
            PreferencesKeys::LORA_CR,
            config.codingRate
        ) > 0
    );

    ok &= (
        nvs.putUChar(
            PreferencesKeys::LORA_POWER,
            config.power
        ) > 0
    );

    ok &= (
        nvs.putUChar(
            PreferencesKeys::LORA_SYNC,
            config.syncWord
        ) > 0
    );

    ok &= (
        nvs.putUShort(
            PreferencesKeys::LORA_PRE,
            config.preambleLength
        ) > 0
    );

    nvs.end();

    return ok;
}

bool PreferencesManager::saveAll(
    const SystemConfig& system,
    const ImuOffsets& offsets)
{
    bool ok = true;

    ok &= saveSystem(system);

    ok &= saveOffsets(offsets);

    ok &= saveLora(system.lora);

    return ok;
}

/*
==========================================================
                Restore Defaults
==========================================================
*/

bool PreferencesManager::restoreDefaults()
{
    Preferences nvs;

    if(!openNamespace(nvs, false))
        return false;

    nvs.clear();

    nvs.end();

    SystemConfig system;

    ImuOffsets offsets;

    return saveAll(
        system,
        offsets
    );
}

/*
==========================================================
                    Debug
==========================================================
*/

void PreferencesManager::printAll(
    Stream& stream)
{
    Preferences nvs;

    if(!openNamespace(nvs, true))
        return;

    stream.println();
    stream.println("========== Preferences ==========");

    stream.println();

    stream.println("----- Offsets -----");

    stream.print("Pitch: ");
    stream.println(
        nvs.getFloat(
            PreferencesKeys::PITCH,
            0.0f
        )
    );

    stream.print("Roll : ");
    stream.println(
        nvs.getFloat(
            PreferencesKeys::ROLL,
            0.0f
        )
    );

    stream.print("Yaw  : ");
    stream.println(
        nvs.getFloat(
            PreferencesKeys::YAW,
            0.0f
        )
    );

    stream.println();

    stream.println("----- System -----");

    stream.print("Battery Limit: ");
    stream.println(
        nvs.getFloat(
            PreferencesKeys::BATTERY_LIMIT,
            7.5f
        )
    );

    stream.print("Telemetry (ms): ");
    stream.println(
        nvs.getUShort(
            PreferencesKeys::TELEMETRY_MS,
            100
        )
    );

    stream.print("Flight Time (min): ");
    stream.println(
        nvs.getUShort(
            PreferencesKeys::FLIGHT_TIME,
            30
        )
    );

    stream.print("WiFi Enabled: ");
    stream.println(
        nvs.getBool(
            PreferencesKeys::WIFI_ENABLED,
            true
        )
    );

    stream.println();

    stream.println("----- LoRa -----");

    stream.print("Frequency: ");
    stream.println(
        nvs.getFloat(
            PreferencesKeys::LORA_FREQ,
            915.0f
        )
    );

    stream.print("Bandwidth: ");
    stream.println(
        nvs.getFloat(
            PreferencesKeys::LORA_BW,
            125.0f
        )
    );

    stream.print("Spreading Factor: ");
    stream.println(
        nvs.getUChar(
            PreferencesKeys::LORA_SF,
            10
        )
    );

    stream.print("Coding Rate: ");
    stream.println(
        nvs.getUChar(
            PreferencesKeys::LORA_CR,
            5
        )
    );

    stream.print("Power: ");
    stream.println(
        nvs.getUChar(
            PreferencesKeys::LORA_POWER,
            20
        )
    );

    stream.print("Sync Word: 0x");
    stream.println(
        nvs.getUChar(
            PreferencesKeys::LORA_SYNC,
            0x12
        ),
        HEX
    );

    stream.print("Preamble: ");
    stream.println(
        nvs.getUShort(
            PreferencesKeys::LORA_PRE,
            8
        )
    );

    stream.println();
    stream.println("=================================");

    nvs.end();
}