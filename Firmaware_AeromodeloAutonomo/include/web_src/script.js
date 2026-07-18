"use strict";

const $ = id => document.getElementById(id);

const content = $("content");
const menu = $("menu");
const menuButton = $("menuButton");
const overlay = $("overlay");

let currentPage = "";
let statusTimer = null;
let lastStatus = null;
/* ---------- HTTP helpers ---------- */

async function GET(url){
  const r = await fetch(url);
  if(!r.ok) throw new Error(r.status);
  return r;
}
const GET_JSON = async url => (await GET(url)).json();
const GET_TEXT = async url => (await GET(url)).text();

async function POST(url, data = null){
  const r = await fetch(url, { method:"POST", body:data });
  const json = await r.json();
  if(!r.ok) throw json;
  return json;
}
async function checkSd(){

    try{

        const json = await POST("/api/checksd");

        alertSuccess(json);

        await loadStatus();

    }
    catch(error){

        alertError(error);

    }
}
/* ---------- DOM helpers ---------- */

function text(id, value){
  const e = $(id);
  if(e) e.textContent = value;
}

function valueField(id, v){
  const e = $(id);
  if(!e) return "";
  if(v === undefined) return e.value;
  e.value = v;
}

function checked(id, v){
  const e = $(id);
  if(!e) return false;
  if(v === undefined) return e.checked;
  e.checked = v;
}

function led(id, state){
  const e = $(id);
  if(!e) return;
  e.classList.toggle("ok", state);
  e.classList.toggle("error", !state);
}
function setGpsLed(status){

    const e = $("gpsLed");

    if(!e) return;

    e.classList.remove(
        "gps-red",
        "gps-orange",
        "gps-yellow",
        "gps-green"
    );

    switch(status){

        case 0:
            e.classList.add("gps-red");
            break;

        case 1:
            e.classList.add("gps-orange");
            break;

        case 2:
            e.classList.add("gps-yellow");
            break;

        case 3:
            e.classList.add("gps-green");
            break;
    }
}
function num(v, digits = 2){
  return Number(v ?? 0).toFixed(digits);
}

function alertError(error){
  console.error(error);
  alert("Erro de comunicação.");
}

function alertSuccess(json){
  if(json && json.message) alert(json.message);
}

/* ---------- Menu ---------- */

function closeMenu(){
  menu.classList.remove("open");
  overlay.classList.remove("open");
}

menuButton.onclick = () => {
  menu.classList.toggle("open");
  overlay.classList.toggle("open");
};

overlay.onclick = closeMenu;

document.querySelectorAll("#menu button").forEach(button => {
  button.onclick = () => loadPage(button.dataset.page);
});

/* ---------- Page loading ---------- */

async function loadPage(page){
  if(currentPage === page){
    closeMenu();
    return;
  }

  stopStatus();
  stopConsole();

  try{
    content.innerHTML = await GET_TEXT("/page/" + page);
    currentPage = page;
    initializePage(page);
    closeMenu();
  }
  catch(error){
    alertError(error);
  }
}

function initializePage(page){
  switch(page){

    case "status":
      initializeLiveStatus();
      break;

    case "console":
      initializeConsole();
      break;

    case "logs":
      loadLogs();
      $("deleteAllLogs")?.addEventListener("click", deleteAllLogs);
      break;

    case "offsets":

    loadConfig();
    loadStatus();

    $("refreshImu")?.addEventListener(
        "click",
        loadStatus
    );

    $("zeroPitch")?.addEventListener(
        "click",
        () => tareAxis("pitch")
    );

    $("zeroRoll")?.addEventListener(
        "click",
        () => tareAxis("roll")
    );

    $("zeroYaw")?.addEventListener(
        "click",
        () => tareAxis("yaw")
    );

    $("saveOffsets")?.addEventListener(
        "click",
        async () => {
            await save("offset");

            // O offset novo so passa a valer depois do firmware
            // processar o evento (loop de ~100ms) -- espera um
            // pouco e atualiza a leitura exibida, pra proxima tara
            // ja partir de um valor fresco em vez do antigo.
            setTimeout(loadStatus, 300);
        }
    );

    break;

    case "pid":

    loadConfig();

    $("savePid")?.addEventListener(
        "click",
        () => save("pid")
    );

    break;

    case "system":

    loadConfig();

    $("saveSystem")?.addEventListener(
        "click",
        () => save("system")
    );

    break;

    case "lora":

    loadConfig();

    $("saveLora")?.addEventListener(
        "click",
        () => save("lora")
    );

    break;

    case "flight":

    loadStatus();

    $("startFlight")?.addEventListener(
        "click",
        startFlight
    );

    $("endFlight")?.addEventListener(
        "click",
        endFlight
    );

    $("restartESP")?.addEventListener(
        "click",
        restartESP
    );

    break;
    }
}
let tareBusy = false;
async function tareAxis(axis){
    // Busca um /api/status fresco na hora do clique: o offset
    // realmente ativo no firmware e a leitura sob esse mesmo
    // offset vem do MESMO pacote, entao o calculo nunca soma em
    // cima de um clique anterior nem ignora o offset atual --
    // ele sempre recalcula o angulo bruto do zero:
    // bruto = offsetAtivoNoFirmware + leituraAtualSobEsseOffset.
    //
    // tareBusy trava cliques repetidos enquanto uma tara ainda
    // esta em andamento (evita disparar varias buscas ao mesmo
    // tempo se clicar rapido demais).
    if(tareBusy) return;
    tareBusy = true;

    try{
        const data = await GET_JSON("/api/status");

        const offsetKey = axis + "Offset";
        const bruto = Number(data[offsetKey] ?? 0) + Number(data[axis] ?? 0);

        valueField(axis + "Offset", bruto.toFixed(2));

        fillStatus(data);
    }
    catch(error){
        alertError(error);
    }
    finally{
        tareBusy = false;
    }
}
/* ---------- Console ---------- */

let consoleTimer = null;

function initializeConsole(){

    loadConsole();

    const auto = $("autoStatus");

    if(auto){

        auto.onchange = () => {

            if(auto.checked)
                startConsole();
            else
                stopConsole();

        };

        if(auto.checked)
            startConsole();
    }

    $("refreshTelemetry")?.addEventListener(
        "click",
        loadConsole
    );
}

function startConsole(){

    stopConsole();

    consoleTimer = setInterval(
        loadConsole,
        500
    );
}

function stopConsole(){

    if(consoleTimer == null)
        return;

    clearInterval(consoleTimer);

    consoleTimer = null;
}

async function loadConsole(){

    try{

        const json = await GET_TEXT(
            "/api/telemetry"
        );

        text(
            "telemetryJson",
            json
        );

    }
    catch(error){

        text(
            "telemetryJson",
            "Erro de comunicação."
        );

    }
}
/* ---------- Live status (página status) ---------- */

function initializeLiveStatus(){
  loadStatus();

  const auto = $("autoStatus");
  if(auto){
    auto.onchange = () => (auto.checked ? startStatus() : stopStatus());
    if(auto.checked) startStatus();
  }

  $("refreshStatus")?.addEventListener("click", loadStatus);
  $("checkSd")?.addEventListener("click", checkSd);
}

function startStatus(){
  stopStatus();
  statusTimer = setInterval(loadStatus, 500);
}

function stopStatus(){
  if(statusTimer == null) return;
  clearInterval(statusTimer);
  statusTimer = null;
}

async function loadStatus(){
  try{
    fillStatus(await GET_JSON("/api/status"));
  }
  catch(error){
    console.error(error);
  }
}

const STATUS_TEXT_FIELDS = {
  pitch: d => num(d.pitch),
  roll: d => num(d.roll),
  yaw: d => num(d.yaw),
  latitude: d => num(d.lat, 7),
  longitude: d => num(d.lon, 7),
  gpsAltitude: d => num(d.gpsAlt),
  baroAltitude: d => num(d.baroAlt),
  battery: d => num(d.battery),
  satellites: d => d.gpsSat,
  telemetryPeriod: d => d.telemetryMs,
  batteryLimit: d => num(d.batteryLimit),
  wifiClients: d => d.wifiClients,
  pitchOffset: d => num(d.pitchOffset),
  rollOffset: d => num(d.rollOffset),
  yawOffset: d => num(d.yawOffset),
  flightMode: d => d.flightMode,
};

function fillStatus(data){
  for(const [id, fn] of Object.entries(STATUS_TEXT_FIELDS)){
    text(id, fn(data));
  }

  led("imuLed", data.imuOk);
  led("bmpLed", data.bmpOk);
  led("loraLed", data.loraOk);
  led("sdLed", data.sdOk);

  setGpsLed(data.gpsStatus);

  updateCountdownUi(data.flightMode);

  text("pitchNow", num(data.pitch));
  text("rollNow", num(data.roll));
  text("yawNow", num(data.yaw));
  lastStatus = data;
}

/* ---------- Configuration (offsets / system / lora) ---------- */

const CONFIG_FIELDS = {
  pitchOffset: "pitchOffset",
  rollOffset: "rollOffset",
  yawOffset: "yawOffset",

  pitchKp: "pitchKp",
  pitchKi: "pitchKi",
  pitchKd: "pitchKd",

  rollKp: "rollKp",
  rollKi: "rollKi",
  rollKd: "rollKd",

  yawKp: "yawKp",
  yawKi: "yawKi",
  yawKd: "yawKd",

  batteryLimit: "batteryLimit",

  telemetryPeriod: "telemetryMs",
  lowBatteryTelemetryPeriod: "lowBatteryTelemetryMs",

  frequency: "frequency",
  bandwidth: "bandwidth",
  spreadingFactor: "spreadingFactor",
  codingRate: "codingRate",
  power: "power",
  preambleLength: "preambleLength",
};

async function loadConfig(){
  try{
    fillConfig(await GET_JSON("/api/config"));
  }
  catch(error){
    alertError(error);
  }
}

function fillConfig(cfg){

    for(const [id, key] of Object.entries(CONFIG_FIELDS))
        valueField(id, cfg[key]);

    checked(
        "preFlightTelemetryEnabled",
        cfg.preFlightTelemetryEnabled
    );

    checked(
        "telemetryWeb",
        cfg.telemetryWeb
    );

    valueField(
        "syncWord",
        Number(cfg.syncWord).toString(16).toUpperCase()
    );
}

const SAVE_FORMS = {
  offset: {
    url: "/api/offsets",
    fields: { pitch:"pitchOffset", roll:"rollOffset", yaw:"yawOffset" },
  },
  pid: {
    url: "/api/pid",
    fields: {
      pitchKp:"pitchKp", pitchKi:"pitchKi", pitchKd:"pitchKd",
      rollKp:"rollKp", rollKi:"rollKi", rollKd:"rollKd",
      yawKp:"yawKp", yawKi:"yawKi", yawKd:"yawKd",
    },
  },
  system: {
      url: "/api/system",
      fields: {
          battery: "batteryLimit",
          telemetry: "telemetryPeriod",
          lowBatteryTelemetry: "lowBatteryTelemetryPeriod",
      },
      checks: {
          preFlightTelemetry: "preFlightTelemetryEnabled",
          web: "telemetryWeb",
      },
  },
  lora: {
    url: "/api/lora",
    fields: {
      freq:"frequency", bw:"bandwidth", sf:"spreadingFactor",
      cr:"codingRate", power:"power", sync:"syncWord", pre:"preambleLength",
    },
  },
};
async function save(type){

    const form = SAVE_FORMS[type];
    const data = new URLSearchParams();

    for(const [param, id] of Object.entries(form.fields))
        data.append(param, valueField(id));

    if(form.checks){

        for(const [param, id] of Object.entries(form.checks))
            data.append(param, checked(id) ? 1 : 0);

    }

    try{

        alertSuccess(await POST(form.url, data));

        if(type === "offset"){

            valueField("pitchOffset", "0");
            valueField("rollOffset", "0");
            valueField("yawOffset", "0");

            await loadStatus();
        }

    }
    catch(error){

        alertError(error);

    }
}
/* ---------- Flight control ---------- */

const COUNTDOWN_DURATION_S = 10;
let countdownActive = false;
let countdownTimer = null;

// A contagem aqui e so visual: o firmware manda o estado COUNTDOWN
// assim que /api/start e aceito, e quem decide quando vira FLIGHT de
// verdade e o seu codigo no main.cpp. Essa UI so mostra um numero
// decrescendo enquanto o polling de /api/status continuar reportando
// "COUNTDOWN", e some sozinha assim que o modo mudar (pra FLIGHT ou
// de volta pra CONFIG) -- por isso nunca fica "dessincronizada" por
// muito tempo, mesmo que COUNTDOWN_DURATION_S nao bata exatamente
// com a duracao real da contagem no firmware.
function updateCountdownUi(mode){
    const card = $("countdownCard");

    if(!card)
        return;

    if(mode === "COUNTDOWN"){

        if(countdownActive)
            return;

        countdownActive = true;
        card.style.display = "";

        let secondsLeft = COUNTDOWN_DURATION_S;
        text("countdownNumber", secondsLeft);

        countdownTimer = setInterval(() => {

            secondsLeft--;
            text("countdownNumber", Math.max(secondsLeft, 0));

            if(secondsLeft <= 0){
                clearInterval(countdownTimer);
                countdownTimer = null;
            }

        }, 1000);

        return;
    }

    if(countdownActive){
        countdownActive = false;

        if(countdownTimer){
            clearInterval(countdownTimer);
            countdownTimer = null;
        }

        card.style.display = "none";
    }
}

async function startFlight(){

    const problems = [];

    if(lastStatus){

        if(!lastStatus.imuOk)
            problems.push("• IMU");

        if(!lastStatus.bmpOk)
            problems.push("• BMP280");

        if(!lastStatus.loraOk)
            problems.push("• LoRa");

        if(!lastStatus.sdOk)
            problems.push("• Cartão SD");

        switch(lastStatus.gpsStatus){

            case 0:
                problems.push("• GPS sem comunicação");
                break;

            case 1:
                problems.push("• GPS sem FIX");
                break;

            case 2:
                problems.push("• GPS com FIX insuficiente");
                break;
        }
    }

    if(problems.length){

        const msg =
            "Os seguintes módulos apresentam problemas:\n\n" +
            problems.join("\n") +
            "\n\nDeseja continuar mesmo assim?";

        if(!confirm(msg))
            return;
    }

    if(!confirm(
        "Iniciar voo?\n\nAs configurações serão bloqueadas até reiniciar."
    ))
        return;

    try{
        alertSuccess(await POST("/api/start"));
        await loadStatus();
    }
    catch(error){
        alertError(error);
    }
}

async function endFlight(){
    if(!confirm(
        "Finalizar voo?\n\nO modo passará para LANDED."
    ))
        return;

    try{
        alertSuccess(await POST("/api/end"));
        await loadStatus();
    }
    catch(error){
        alertError(error);
    }
}

async function restartESP(){
  if(!confirm("Reiniciar ESP32?")) return;

  try{
    alertSuccess(await POST("/api/restart"));
  }
  catch(error){
    alertError(error);
  }
}
/* ---------- Logs ---------- */

async function loadLogs(){

    try{

        // /api/logs devolve um array JSON puro (ex: ["voo1.csv"]),
        // nao um objeto {files:[...]}. Usar json.files aqui sempre
        // dava undefined e quebrava fillLogs() antes de preencher
        // a tabela -- por isso a pagina nunca listava nada.
        const files = await GET_JSON("/api/logs");

        fillLogs(files);

    }
    catch(error){

        alertError(error);

    }
}

function fillLogs(files){

    files = files || [];

    const table = $("logsTable");

    if(!table)
        return;

    if(!files.length){

        table.innerHTML = `
            <tr>
                <td colspan="2">
                    Nenhum log encontrado.
                </td>
            </tr>
        `;

        return;
    }

    table.innerHTML = "";

    for(const file of files){

        table.insertAdjacentHTML(
            "beforeend",

            `
            <tr>

                <td>${file}</td>

                <td>

                    <button class="secondary" onclick="downloadLog('${file}')">
                        Baixar
                    </button>

                    <button class="secondary" onclick="renameLog('${file}')">
                        Renomear
                    </button>

                    <button
                        class="danger"
                        onclick="deleteLog('${file}')">

                        Excluir

                    </button>

                </td>

            </tr>
            `
        );
    }
}

function downloadLog(file){

    window.location =
        "/api/logs/download?file=" +
        encodeURIComponent(file);

}

async function renameLog(file){

    let newName = prompt(
        "Novo nome para \"" + file + "\":",
        file
    );

    if(newName === null)
        return;

    newName = newName.trim();

    if(!newName || newName === file)
        return;

    if(!newName.toLowerCase().endsWith(".csv"))
        newName += ".csv";

    const data = new URLSearchParams();

    data.append("file", file);
    data.append("newName", newName);

    try{

        alertSuccess(
            await POST(
                "/api/logs/rename",
                data
            )
        );

        loadLogs();

    }
    catch(error){

        alertError(error);

    }
}

async function deleteLog(file){

    if(!confirm(
        "Excluir " + file + " ?"
    ))
        return;

    const data = new URLSearchParams();

    data.append(
        "file",
        file
    );

    try{

        alertSuccess(
            await POST(
                "/api/logs/delete",
                data
            )
        );

        loadLogs();

    }
    catch(error){

        alertError(error);

    }
}
async function deleteAllLogs(){

    if(!confirm(
        "Excluir TODOS os logs?"
    ))
        return;

    try{

        alertSuccess(
            await POST(
                "/api/logs/deleteAll"
            )
        );

        loadLogs();

    }
    catch(error){

        alertError(error);

    }
}

/* ---------- Boot ---------- */

window.addEventListener("load", () => loadPage("status"));
