#include <WiFi.h>
#include <WebServer.h>

extern String generarFecha();
extern String generarDatosJSON();
extern void handleRoot();
extern void handleDatos();
extern void handleSetUmbral();



const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>ESP32 Panel - Dracula Theme</title>
<style>
  body {
    background-color: #282a36;
    color: #f8f8f2;
    font-family: 'Consolas', monospace;
    margin: 20px;
  }
  h2, h3 { color: #bd93f9; }
  .card {
    background-color: #44475a;
    padding: 15px;
    border-radius: 10px;
    margin-bottom: 20px;
    box-shadow: 0 0 10px #6272a4;
  }
  input, button {
    background-color: #6272a4;
    color: #f8f8f2;
    border: none;
    padding: 8px 12px;
    border-radius: 5px;
    font-size: 16px;
  }
  button:hover { background-color: #50fa7b; color: #282a36; }
  table {
    width: 100%;
    border-collapse: collapse;
    margin-top: 10px;
  }
  th {
    background-color: #6272a4;
    color: #f8f8f2;
  }
  th, td {
    border: 1px solid #6272a4;
    padding: 8px;
    text-align: center;
  }
  tr:nth-child(even) {
    background-color: #3b3e53;
  }
</style>
</head>
<body>

<h2>🌙 Panel de Control ESP32 (Dracula)</h2>

<div class="card">
  <h3>1️⃣ Control de Umbral de Pasos</h3>
  <p>Valor actual: <span id="valorUmbral">--</span></p>
  <input type="number" id="nuevoUmbral" placeholder="Nuevo umbral">
  <button onclick="enviarUmbral()">Enviar</button>
</div>

<div class="card">
  <h3>2️⃣ Registro de Pasos</h3>
  <table id="tablaPasos">
    <thead><tr><th>Fecha</th><th>Pasos</th></tr></thead>
    <tbody></tbody>
  </table>
</div>

<div class="card">
  <h3>3️⃣ Registro de Temperatura</h3>
  <table id="tablaTemp">
    <thead><tr><th>Fecha-Hora</th><th>Temperatura (°C)</th></tr></thead>
    <tbody></tbody>
  </table>
</div>

<script>
async function obtenerDatos(){
  const res = await fetch('/datos');
  const data = await res.json();

  document.getElementById('valorUmbral').innerText = data.umbralPasos;

  // 更新表格
  const pasosT = document.querySelector('#tablaPasos tbody');
  pasosT.innerHTML = `<tr><td>${data.pasos.fecha}</td><td>${data.pasos.valor}</td></tr>`;

  const tempT = document.querySelector('#tablaTemp tbody');
  tempT.innerHTML = `<tr><td>${data.temp.fecha}</td><td>${data.temp.valor.toFixed(2)}</td></tr>`;
}

function enviarUmbral(){
  const nuevo = document.getElementById('nuevoUmbral').value;
  if(nuevo){
    fetch(`/setUmbral?valor=${nuevo}`).then(()=>obtenerDatos());
  }
}

// 每5秒刷新一次数据
setInterval(obtenerDatos, 5000);
window.onload = obtenerDatos;
</script>

</body>
</html>
)rawliteral";