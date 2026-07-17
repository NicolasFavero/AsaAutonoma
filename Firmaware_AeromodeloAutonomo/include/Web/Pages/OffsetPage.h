#pragma once
#include <Arduino.h>

namespace Web::Pages
{

inline constexpr char OFFSET_PAGE[] PROGMEM = R"rawliteral(

<section>

<h2>Offsets da IMU</h2>

<div class="card">

<h3>Correção</h3>

<table>

<tr>
<td>Pitch (°)</td>
<td><input id="pitchOffset" type="number" step="0.01"></td>
</tr>

<tr>
<td>Roll (°)</td>
<td><input id="rollOffset" type="number" step="0.01"></td>
</tr>

<tr>
<td>Yaw (°)</td>
<td><input id="yawOffset" type="number" step="0.01"></td>
</tr>

</table>

</div>

<div class="card">

<button
id="saveOffsets"
class="primary">

Salvar Offsets

</button>

</div>

</section>

)rawliteral";
}