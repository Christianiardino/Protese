#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <defines.h>

const char* ssid = "PROTESE_WEB_SERVER";
const char* password = "12345678";

IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);

// HTML com graficos em Canvas e sem acentos
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Monitor Protese</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 0; padding: 20px; background-color: #f4f4f4; }
    h2 { color: #333; margin-bottom: 20px; }
    
    /* Layout Horizontal */
    .container { 
      display: flex; 
      flex-direction: row; 
      flex-wrap: wrap; 
      justify-content: center; 
      gap: 20px; 
      align-items: flex-start;
    }
    
    .card { 
      background: white; 
      padding: 15px; 
      border-radius: 10px; 
      box-shadow: 0 4px 8px rgba(0,0,0,0.1); 
      width: 400px; /* Largura fixa para caber os graficos */
    }

    /* Estilo dos Canvas */
    canvas {
      width: 100%;
      height: 200px;
      border: 1px solid #eee;
      background-color: #fafafa;
    }

    .legend {
      font-size: 12px;
      margin-top: 5px;
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 10px;
    }
    .legend-item { display: flex; align-items: center; }
    .color-box { width: 10px; height: 10px; margin-right: 5px; display: inline-block; }

    .controls { margin-bottom: 20px; padding: 10px; background: #e2e2e2; border-radius: 8px; display: inline-block;}
  </style>
</head>
<body>
  <h2>Monitoramento da Protese</h2>

  <div class="controls">
    <label>Atualizar (ms): </label>
    <input type="number" id="updateRate" value="100" min="50" step="50" style="width: 60px;">
    <button onclick="updateTimer()">Aplicar</button>
  </div>

  <div class="container">
    
    <div class="card">
      <h3>Sensor Fotoeletrico</h3>
      <canvas id="canvasFoto"></canvas>
      <div class="legend" id="legendFoto"></div>
    </div>

    <div class="card">
      <h3>Corrente Motores</h3>
      <canvas id="canvasCorrente"></canvas>
      <div class="legend" id="legendCorrente"></div>
    </div>

    <div class="card">
      <h3>PWM Motores</h3>
      <canvas id="canvasPWM"></canvas>
      <div class="legend" id="legendPWM"></div>
    </div>

  </div>

  <script>
    var intervalId;
    
    // Configuracoes dos Graficos
    const maxPoints = 50; // Pontos no eixo X
    
    // Estrutura de Cores e Nomes
    const colors = ["#FF0000", "#00FF00", "#0000FF", "#FFA500", "#800080", "#00FFFF"];
    const fingerNames = ["Polegar", "Indicador", "Medio", "Anelar", "Mindinho", "M6"];
    const sensorNames = ["Sensor 1", "Sensor 2", "Sensor 3", "Sensor 4"];

    // Historico de dados para os graficos
    var historyFoto = [[], [], [], []];
    var historyCorrente = [[], [], [], [], []];
    var historyPWM = [[], [], [], [], [], []];

    function initLegends() {
      createLegend("legendFoto", sensorNames, 4);
      createLegend("legendCorrente", fingerNames, 5);
      createLegend("legendPWM", fingerNames, 6);
    }

    function createLegend(elementId, names, count) {
      let html = "";
      for(let i=0; i<count; i++) {
        html += `<div class='legend-item'><span class='color-box' style='background:${colors[i]}'></span>${names[i]}</div>`;
      }
      document.getElementById(elementId).innerHTML = html;
    }

    function drawChart(canvasId, dataHistory, maxY) {
      const canvas = document.getElementById(canvasId);
      const ctx = canvas.getContext('2d');
      const width = canvas.width = canvas.offsetWidth;
      const height = canvas.height = canvas.offsetHeight;

      ctx.clearRect(0, 0, width, height);
      
      // Desenha linhas de grade
      ctx.strokeStyle = "#e0e0e0";
      ctx.beginPath();
      for(let i=0; i<=4; i++) {
        let y = i * (height/4);
        ctx.moveTo(0, y); ctx.lineTo(width, y);
      }
      ctx.stroke();

      // Desenha os dados
      const stepX = width / maxPoints;

      for (let lineIdx = 0; lineIdx < dataHistory.length; lineIdx++) {
        const lineData = dataHistory[lineIdx];
        if (lineData.length < 2) continue;

        ctx.strokeStyle = colors[lineIdx];
        ctx.lineWidth = 2;
        ctx.beginPath();

        for (let i = 0; i < lineData.length; i++) {
          let val = lineData[i];
          // Escala Y (Invertido porque Canvas 0,0 eh topo-esquerda)
          let y = height - ((val / maxY) * height);
          let x = i * stepX;
          
          if (i===0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();
      }
    }

    function fetchData() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var data = JSON.parse(this.responseText);
          
          // Atualiza Historicos (FIFO)
          updateHistory(historyFoto, data.foto);
          updateHistory(historyCorrente, data.corrente);
          updateHistory(historyPWM, data.pwm);

          // Redesenha (ajuste os valores maximos maxY conforme necessidade do seu sensor)
          // Ex: Foto vai ate ~1024? 4096? Ajustei para 1024 como base.
          // Corrente vai ate ~4096?
          // PWM vai ate 255
          drawChart("canvasFoto", historyFoto, 500);     
          drawChart("canvasCorrente", historyCorrente, 4096); 
          drawChart("canvasPWM", historyPWM, 260); 
        }
      };
      xhr.open("GET", "/data", true);
      xhr.send();
    }

    function updateHistory(historyMatrix, newDataArray) {
      for(let i=0; i<historyMatrix.length; i++) {
        if(newDataArray[i] !== undefined) {
          historyMatrix[i].push(newDataArray[i]);
          if(historyMatrix[i].length > maxPoints) {
            historyMatrix[i].shift();
          }
        }
      }
    }

    function updateTimer() {
        var ms = document.getElementById('updateRate').value;
        if (ms < 50) ms = 50; 
        if(intervalId) clearInterval(intervalId);
        intervalId = setInterval(fetchData, ms);
    }

    // Inicializacao
    initLegends();
    updateTimer();

  </script>
</body>
</html>)rawliteral";

void startWebServer() {
    WiFi.softAPConfig(local_ip, gateway, subnet);
    bool success = WiFi.softAP(ssid, password, 1, 0, 4);

    if (success && DEBUG_PRINT) {
        Serial.println("Hotspot Ativado.");
        Serial.print("IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Falha Soft-AP.");
    }

    // Rota Principal
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", index_html);
    });

    // Rota JSON Data
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
        String json = "{";

        // Dedos (Treino) - Nao solicitado no grafico, mas mantido no JSON caso precise
        json += "\"dedos\":[";
        for (int i = 0; i < 5; i++) {
            json += String(uiVetorDedosTreino[i]);
            if (i < 4) json += ",";
        }
        json += "],";

        // Fotoeletrico
        json += "\"foto\":[";
        for (int i = 0; i < 4; i++) {
            json += String(dArrDadosSensorFoto[i], 2);
            if (i < 3) json += ",";
        }
        json += "],";

        // Corrente
        json += "\"corrente\":[";
        for (int i = 0; i < 5; i++) {
            json += String(dArrSensorCorrente[i], 0);
            if (i < 4) json += ",";
        }
        json += "],";

        // PWM
        json += "\"pwm\":[";
        for (int i = 0; i < 6; i++) {
            json += String(uiArrPwmRampa[i]);
            if (i < 5) json += ",";
        }
        json += "]";

        json += "}";

        request->send(200, "application/json", json);
    });

    server.begin();
}