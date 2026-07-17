#pragma once
#include <Arduino.h>

namespace Web::Pages
{

inline constexpr char SYSTEM_PAGE[] PROGMEM = R"rawliteral(

<section>

<h2>Sistema</h2>

<div class="card">

<h3>Configurações Gerais</h3>

<table>

<tr>
<td>Limite da bateria (V)</td>
<td>
<input
id="batteryLimit"
type="number"
step="0.01">
</td>
</tr>

<tr>
<td>Telemetria (ms)</td>
<td>
<input
id="telemetryPeriod"
type="number">
</td>
</tr>

<tr>
<td>Tempo de voo (s)</td>
<td>
<input
id="flightTime"
type="number">
</td>
</tr>

<tr>
<td>WiFi habilitado</td>
<td>
<input
id="wifiEnabled"
type="checkbox">
</td>
</tr>

<tr>
<td>Telemetria Web</td>
<td>
<input
id="telemetryWeb"
type="checkbox">
</td>
</tr>

</table>

</div>

<br>

<button
class="primary"
id="saveSystem">

Salvar Configurações

</button>

</section>

)rawliteral";

}