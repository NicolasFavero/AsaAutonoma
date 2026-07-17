#pragma once
#include <Arduino.h> 
namespace Web::Pages{

inline constexpr char STATUS_PAGE[] PROGMEM = R"rawliteral(

<section>

<h2>Status do Sistema</h2>

<div style="margin-bottom:15px">

<button
class="primary"
id="refreshStatus">

Atualizar Agora

</button>

<label style="margin-left:20px">

<input
type="checkbox"
id="autoStatus">

Atualização automática

</label>

</div>

<div class="card">

<h3>Modo de Operação</h3>

<table>

<tr>
<td>Modo</td>
<td id="flightMode">-</td>
</tr>

<tr>
<td>Clientes WiFi</td>
<td id="wifiClients">-</td>
</tr>

</table>

</div>

<div class="card">

<h3>Atitude</h3>

<table>

<tr>
<td>Pitch</td>
<td id="pitch">-</td>
</tr>

<tr>
<td>Roll</td>
<td id="roll">-</td>
</tr>

<tr>
<td>Yaw</td>
<td id="yaw">-</td>
</tr>

</table>

</div>

<div class="card">

<h3>GPS</h3>

<table>

<tr>
<td>Latitude</td>
<td id="latitude">-</td>
</tr>

<tr>
<td>Longitude</td>
<td id="longitude">-</td>
</tr>

<tr>
<td>Altitude GPS</td>
<td id="gpsAltitude">-</td>
</tr>

<tr>
<td>Satélites</td>
<td id="satellites">-</td>
</tr>

</table>

</div>

<div class="card">

<h3>Barômetro</h3>

<table>

<tr>
<td>Altitude</td>
<td id="baroAltitude">-</td>
</tr>

</table>

</div>

<div class="card">

<h3>Energia</h3>

<table>

<tr>
<td>Bateria</td>
<td id="battery">-</td>
</tr>

</table>

</div>

<div class="card">

<h3>Sensores</h3>

<table>

<tr>
<td>IMU</td>
<td id="imuLed">●</td>
</tr>

<tr>
<td>GPS</td>
<td id="gpsLed">●</td>
</tr>

<tr>
<td>BMP280</td>
<td id="bmpLed">●</td>
</tr>

</table>

</div>

</section>

)rawliteral";

}