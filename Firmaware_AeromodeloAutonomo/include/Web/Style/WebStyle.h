#pragma once

#include <Arduino.h>

namespace Web::Style
{

inline constexpr char CSS[] PROGMEM = R"rawliteral(

:root{

--primary:#1565c0;
--primaryDark:#0d47a1;
--success:#18c218;
--danger:#d62828;

--background:#edf1f5;
--card:#ffffff;
--border:#d9dde3;

--text:#222;
--text2:#666;

--radius:14px;

--shadow:0 4px 12px rgba(0,0,0,.10);

}

*{

margin:0;
padding:0;
box-sizing:border-box;
font-family:Arial,Helvetica,sans-serif;

}

body{

background:var(--background);
color:var(--text);

}

header{

height:60px;

display:flex;

align-items:center;

gap:15px;

padding:0 18px;

background:linear-gradient(
90deg,
var(--primaryDark),
var(--primary));

color:white;

box-shadow:0 2px 10px rgba(0,0,0,.18);

position:sticky;

top:0;

z-index:1000;

}

h1{

font-size:22px;
font-weight:600;

flex:1;

}

#menuButton{

border:none;

background:none;

color:white;

font-size:30px;

cursor:pointer;

transition:.25s;

}

#menuButton:hover{

transform:scale(1.1);

}

#menu{

position:fixed;

top:60px;

left:-270px;

width:260px;

height:calc(100% - 60px);

overflow:auto;

background:white;

transition:.25s;

box-shadow:2px 0 16px rgba(0,0,0,.25);

z-index:999;

}

#menu.open{

left:0;

}

#menu button{

width:100%;

padding:15px 20px;

border:none;

background:white;

cursor:pointer;

text-align:left;

font-size:16px;

transition:.2s;

border-left:4px solid transparent;

}

#menu button:hover{

background:#f2f6fb;

border-left:4px solid var(--primary);

padding-left:26px;

}

main{

max-width:1100px;

margin:auto;

padding:25px;

}

section{

animation:fade .25s;

}

@keyframes fade{

from{

opacity:0;

transform:translateY(8px);

}

to{

opacity:1;

transform:none;

}

}

h2{

margin-bottom:20px;

color:var(--primary);

font-size:28px;

font-weight:600;

}

h3{

margin-bottom:14px;

color:var(--primary);

font-size:18px;

}.card{

background:var(--card);

border-radius:var(--radius);

box-shadow:var(--shadow);

padding:20px;

margin-bottom:20px;

border:1px solid var(--border);

transition:.25s;

}

.card:hover{

transform:translateY(-2px);

box-shadow:0 8px 20px rgba(0,0,0,.12);

}

.grid{

display:grid;

grid-template-columns:repeat(auto-fit,minmax(280px,1fr));

gap:20px;

}

table{

width:100%;

border-collapse:collapse;

}

td{

padding:12px 6px;

border-bottom:1px solid #ececec;

}

td:first-child{

font-weight:bold;

color:var(--text);

width:55%;

}

td:last-child{

text-align:right;

color:var(--primary);

font-weight:bold;

}

.value{

font-weight:bold;

color:var(--primary);

}

input,
select{

width:100%;

padding:10px 12px;

border:1px solid var(--border);

border-radius:8px;

font-size:15px;

background:white;

transition:.2s;

}

input:focus,
select:focus{

outline:none;

border-color:var(--primary);

box-shadow:0 0 0 3px rgba(21,101,192,.15);

}

button{

transition:.2s;

}

button.primary{

background:var(--primary);

color:white;

border:none;

padding:11px 20px;

border-radius:8px;

cursor:pointer;

font-size:15px;

font-weight:bold;

}

button.primary:hover{

background:var(--primaryDark);

transform:translateY(-1px);

}

button.danger{

background:var(--danger);

color:white;

border:none;

padding:11px 20px;

border-radius:8px;

cursor:pointer;

font-size:15px;

font-weight:bold;

}

button.danger:hover{

filter:brightness(.92);

transform:translateY(-1px);

}

.buttons{

display:flex;

gap:12px;

flex-wrap:wrap;

margin-top:15px;

}

.center{

text-align:center;

}

.right{

text-align:right;

}

.left{

text-align:left;

}

hr{

border:none;

border-top:1px solid var(--border);

margin:18px 0;

}.status{

display:flex;

align-items:center;

gap:10px;

padding:10px 14px;

border-radius:10px;

background:#f7f9fc;

border:1px solid var(--border);

margin-bottom:18px;

}

.statusItem{

display:flex;

align-items:center;

gap:8px;

font-size:15px;

font-weight:600;

color:var(--text);

}

.led{

display:inline-block;

width:14px;

height:14px;

border-radius:50%;

background:#9e9e9e;

transition:.25s;

}

.led.ok{

background:var(--success);

box-shadow:0 0 10px var(--success);

}

.led.error{

background:var(--danger);

box-shadow:0 0 10px var(--danger);

}

.badge{

display:inline-block;

padding:4px 10px;

border-radius:999px;

font-size:13px;

font-weight:bold;

background:#eaf3ff;

color:var(--primary);

}

.metric{

display:flex;

flex-direction:column;

align-items:center;

justify-content:center;

padding:18px;

border-radius:12px;

background:white;

border:1px solid var(--border);

box-shadow:var(--shadow);

}

.metric h4{

font-size:15px;

font-weight:600;

color:var(--text2);

margin-bottom:12px;

}

.metric span{

font-size:28px;

font-weight:bold;

color:var(--primary);

}

.metric small{

margin-top:6px;

font-size:12px;

color:var(--text2);

}

.metrics{

display:grid;

grid-template-columns:repeat(auto-fit,minmax(170px,1fr));

gap:16px;

margin-bottom:20px;

}

.formRow{

display:grid;

grid-template-columns:220px 1fr;

gap:15px;

align-items:center;

margin-bottom:14px;

}

.formRow label{

font-weight:bold;

}

.checkbox{

display:flex;

align-items:center;

gap:10px;

}

.checkbox input{

width:18px;

height:18px;

}

.notice{

padding:14px;

background:#eef6ff;

border-left:4px solid var(--primary);

border-radius:8px;

margin-bottom:18px;

}

.notice.error{

background:#fff0f0;

border-left-color:var(--danger);

}

.notice.success{

background:#f0fff4;

border-left-color:var(--success);

}