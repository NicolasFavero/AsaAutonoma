#pragma once
#include <Arduino.h>

namespace Web::Pages
{

inline constexpr char FLIGHT_PAGE[] PROGMEM = R"rawliteral(

<section>

<h2>Controle de Voo</h2>

<div class="card">

<h3>Estado Atual</h3>

<table>

<tr>
<td>Modo</td>
<td id="flightMode">CONFIG</td>
</tr>

<tr>
<td>Clientes WiFi</td>
<td id="wifiClients">0</td>
</tr>

<tr>
<td>Bateria</td>
<td id="battery">0.00 V</td>
</tr>

</table>

</div>

<div class="card">

<h3>Comandos</h3>

<p>

Ao iniciar o voo todas as configurações ficam bloqueadas até reiniciar o ESP32.

</p>

<br>

<button
id="startFlight"
class="primary">

Iniciar Voo

</button>

</div>

<div class="card">

<h3>Sistema</h3>

<button
id="restartESP"
class="danger">

Reiniciar ESP32

</button>

</div>

</section>

)rawliteral";

}