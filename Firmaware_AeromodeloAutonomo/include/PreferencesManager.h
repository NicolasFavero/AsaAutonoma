#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "DataTypes.h"

// Cada função abre e fecha a NVS individualmente.
// Isso torna cada operação independente e evita
// manter a namespace aberta desnecessariamente.

class PreferencesManager
{
public:

    bool initialize();

    bool loadSystem(SystemConfig& config);

    bool loadOffsets(ImuOffsets& offsets);

    bool saveSystem(const SystemConfig& config);

    bool saveOffsets(const ImuOffsets& offsets);

    bool restoreDefaults();

    void printAll(Stream& stream);

    bool loadLora(LoraConfig& config);

    bool saveLora(const LoraConfig& config);

    bool loadAll(
        SystemConfig& system,
        ImuOffsets& offsets
    );

    bool saveAll(
        const SystemConfig& system,
        const ImuOffsets& offsets
    );

private:

    Preferences preferences;

    bool openNamespace(
        Preferences& nvs,
        bool readOnly
    );
};