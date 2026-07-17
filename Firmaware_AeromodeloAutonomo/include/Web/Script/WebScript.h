#pragma once

#include <Arduino.h>

namespace Web::Script
{

inline constexpr char JS[] PROGMEM = R"rawliteral(

"use strict";

const $ = id => document.getElementById(id);

const content = $("content");
const menu = $("menu");
const menuButton = $("menuButton");

let currentPage = "";
let statusTimer = null;

async function GET(url)
{
    const r = await fetch(url);

    if(!r.ok)
        throw new Error(r.status);

    return r;
}

async function GET_JSON(url)
{
    return await(await GET(url)).json();
}

async function GET_TEXT(url)
{
    return await (await GET(url)).text();
}

async function POST(url,data=null)
{
    const r = await fetch(url,{
        method:"POST",
        body:data
    });

    const json = await r.json();

    if(!r.ok)
        throw json;

    return json;
}
function text(id,value)
{
    const e=$(id);

    if(e)
        e.textContent=value;
}

function valueField(id,v)
{
    const e=$(id);

    if(!e)
        return "";

    if(v===undefined)
        return e.value;

    e.value=v;
}

function checked(id,v)
{
    const e=$(id);

    if(!e)
        return false;

    if(v===undefined)
        return e.checked;

    e.checked=v;
}

function led(id,state)
{
    const e=$(id);

    if(!e)
        return;

    e.classList.toggle("ok",state);

    e.classList.toggle("error",!state);
}

function alertError(error)
{
    console.error(error);

    alert("Erro de comunicação.");
}

function alertSuccess(json)
{
    if(json.message)
        alert(json.message);
}

menuButton.onclick=()=>
{
    menu.classList.toggle("open");
};

function closeMenu()
{
    menu.classList.remove("open");
}

async function loadPage(page)
{
    if(currentPage===page)
    {
        closeMenu();
        return;
    }

    stopStatus();

    try
    {
        content.innerHTML=
            await GET_TEXT(
                "/page/"+page
            );

        currentPage=page;

        initializePage(page);

        closeMenu();
    }
    catch(error)
    {
        alertError(error);
    }
}

document
.querySelectorAll("#menu button")
.forEach(button=>
{
    button.onclick=()=>
    {
        loadPage(
            button.dataset.page
        );
    };
});

function startStatus()
{
    stopStatus();

    loadStatus();

    statusTimer =
        setInterval(
            loadStatus,
            500
        );
}

function stopStatus()
{
    if(statusTimer==null)
        return;

    clearInterval(
        statusTimer
    );

    statusTimer=null;
}

async function loadStatus()
{
    try
    {
        const data =
            await GET_JSON(
                "/api/status"
            );

        fillStatus(data);
    }
    catch(error)
    {
        console.error(error);
    }
}

function fillStatus(data)
{
    const values =
    {
        pitch:data.pitch.toFixed(2),
        roll:data.roll.toFixed(2),
        yaw:data.yaw.toFixed(2),

        latitude:data.lat.toFixed(7),
        longitude:data.lon.toFixed(7),

        gpsAltitude:data.gpsAlt.toFixed(2),
        baroAltitude:data.baroAlt.toFixed(2),

        battery:data.battery.toFixed(2),

        satellites:data.gpsSat,

        telemetryPeriod:data.telemetryMs,

        batteryLimit:data.batteryLimit.toFixed(2),

        wifiClients:data.wifiClients,

        pitchOffset:data.pitchOffset.toFixed(2),
        rollOffset:data.rollOffset.toFixed(2),
        yawOffset:data.yawOffset.toFixed(2)
    };

    Object.entries(values)
    .forEach(
        ([id,value]) =>
        {
            text(id,value);
        }
    );

    led(
        "imuLed",
        data.imuOk
    );

    led(
        "gpsLed",
        data.gpsOk
    );

    led(
        "bmpLed",
        data.bmpOk
    );

    led(
        "flightLed",
        data.flightMode !== "CONFIG"
    );

    text(
        "flightMode",
        data.flightMode
    );

}
async function save(type)
{
    try
    {
        const data = new URLSearchParams();

        switch(type)
        {
            case "offset":

                data.append(
                    "pitch",
                    valueField("pitchOffset")
                );

                data.append(
                    "roll",
                    valueField("rollOffset")
                );

                data.append(
                    "yaw",
                    valueField("yawOffset")
                );

                alertSuccess(
                    await POST(
                        "/api/offsets",
                        data
                    )
                );

                break;

            case "system":

                data.append(
                    "battery",
                    valueField("batteryLimit")
                );

                data.append(
                    "telemetry",
                    valueField("telemetryPeriod")
                );

                data.append(
                    "flightTime",
                    valueField("flightTime")
                );

                data.append(
                    "wifi",
                    checked("wifiEnabled") ? 1 : 0
                );

                data.append(
                    "web",
                    checked("telemetryWeb") ? 1 : 0
                );

                alertSuccess(
                    await POST(
                        "/api/system",
                        data
                    )
                );

                break;

            case "lora":

                data.append(
                    "freq",
                    valueField("frequency")
                );

                data.append(
                    "bw",
                    valueField("bandwidth")
                );

                data.append(
                    "sf",
                    valueField("spreadingFactor")
                );

                data.append(
                    "cr",
                    valueField("codingRate")
                );

                data.append(
                    "power",
                    valueField("power")
                );

                data.append(
                    "sync",
                    valueField("syncWord")
                );

                data.append(
                    "pre",
                    valueField("preambleLength")
                );

                alertSuccess(
                    await POST(
                        "/api/lora",
                        data
                    )
                );

                break;
        }
    }
    catch(error)
    {
        alertError(error);
    }
}
async function loadConfig()
{
    try
    {
        const cfg =
            await GET_JSON(
                "/api/config"
            );

        fillConfig(cfg);
    }
    catch(error)
    {
        alertError(error);
    }
}

function fillConfig(cfg)
{
    valueField(
        "pitchOffset",
        cfg.pitchOffset
    );

    valueField(
        "rollOffset",
        cfg.rollOffset
    );

    valueField(
        "yawOffset",
        cfg.yawOffset
    );

    valueField(
        "batteryLimit",
        cfg.batteryLimit
    );

    valueField(
        "telemetryPeriod",
        cfg.telemetryMs
    );

    valueField(
        "flightTime",
        cfg.flightTime
    );

    checked(
        "wifiEnabled",
        cfg.wifiEnabled
    );

    checked(
        "telemetryWeb",
        cfg.telemetryWeb
    );

    valueField(
        "frequency",
        cfg.frequency
    );

    valueField(
        "bandwidth",
        cfg.bandwidth
    );

    valueField(
        "spreadingFactor",
        cfg.spreadingFactor
    );

    valueField(
        "codingRate",
        cfg.codingRate
    );

    valueField(
        "power",
        cfg.power
    );

    valueField(
        "syncWord",
        Number(cfg.syncWord)
        .toString(16)
        .toUpperCase()
    );

    valueField(
        "preambleLength",
        cfg.preambleLength
    );
}

function initializeFlight()
{
    $("startFlight")?.addEventListener(
        "click",
        startFlight
    );

    $("restartESP")?.addEventListener(
        "click",
        restartESP
    );
}

async function restartESP()
{
    if(!confirm(
        "Reiniciar ESP32?"
    ))
        return;

    try
    {
        alertSuccess(

            await POST(
                "/api/restart"
            )
        );
    }
    catch(error)
    {
        alertError(error);
    }
}
async function startFlight()
{
    if(!confirm(
        "Iniciar voo?"
    ))
        return;

    try
    {
        alertSuccess(
            await POST(
                "/api/start"
            )
        );
    }
    catch(error)
    {
        alertError(error);
    }
}

function initializePage(page)
{
    switch(page)
    {
        case "status":

            startStatus();

            break;

        case "offsets":

            loadConfig();

            $("saveOffsets")
            ?.addEventListener(
                "click",
                ()=>save("offset")
            );

            break;

        case "system":

            loadConfig();

            $("saveSystem")
            ?.addEventListener(
                "click",
                ()=>save("system")
            );

            break;

        case "lora":

            loadConfig();

            $("saveLora")
            ?.addEventListener(
                "click",
                ()=>save("lora")
            );

            break;

        case "flight":

            initializeFlight();

            break;

        case "diagnostics":

            loadStatus();

            break;
    }
}

window.addEventListener(
    "load",
    ()=>
    {
        loadPage(
            "status"
        );
    }
);

)rawliteral";

}