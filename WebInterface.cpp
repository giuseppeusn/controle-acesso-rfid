#include "WebInterface.h"
#include <WebServer.h>
#include "Estado.h"
#include "RFIDController.h"

extern WebSocketsServer webSocket;

void initWebInterface(WebServer& server) {
  server.on("/", [&server]() {
    String html = R"====(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Controle de acesso RFID</title>
      <style>
        body { font-family: 'Arial', sans-serif; margin: 20px; background-color: #f0f4f8; color: #333; }
        .container { max-width: 800px; margin: auto; background-color: #ffffff; padding: 25px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1); }
        h2, h3 { color: #2c3e50; text-align: center; margin-bottom: 20px; }
        .button-group { text-align: center; margin-bottom: 20px; }
        button {
          display: inline-block;
          padding: 12px 25px;
          margin: 10px;
          border: none;
          border-radius: 8px;
          background-color: #3498db;
          color: white;
          font-size: 16px;
          cursor: pointer;
          transition: background-color 0.3s ease, transform 0.2s ease;
          box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        button:hover { background-color: #2980b9; transform: translateY(-2px); }
        button:active { transform: translateY(0); box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); }
        #log {
          background-color: #ecf0f1; border: 1px solid #bdc3c7; padding: 15px;
          height: 200px; overflow-y: scroll; border-radius: 8px; margin-top: 20px;
          font-family: monospace; color: #2c3e50; white-space: pre-wrap; word-wrap: break-word;
        }
        .success { color: #155724; }
        .error { color: #721c24; }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Controle de acesso RFID</h2>
        <div class="button-group">
          <button onclick="iniciarGravacao()">Cadastrar cartão</button>
          <button onclick="liberarManualmente()">Liberar acesso</button>
        </div>
        <h3>Log de Eventos</h3>
        <div id="log"></div>
        <div id="status"></div>
        <script>
          let ws;

          function connectWebSocket() {
            if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
              return;
            }
            console.log("WebSocket: Tentando conectar...");
            ws = new WebSocket(`ws://${location.hostname}:81/ws`);

            ws.onopen = (event) => {
              console.log("WebSocket: Conectado com sucesso!");
            };

            ws.onmessage = (event) => {
              let logDiv = document.getElementById("log");
              if (!logDiv) return;
              const data = event.data;

              if (data.startsWith("full_log:")) {
                logDiv.innerHTML = data.substring("full_log:".length);
              } else if (data.startsWith("new_log:")) {
                const newLogEntry = data.substring("new_log:".length);
                logDiv.innerHTML += newLogEntry;
                if (newLogEntry.includes("Modo de gravacao")) {
                    updateStatus(newLogEntry.replace(/<br>/g, ''), 'ok');
                }
              } else {
                logDiv.innerHTML += data + "<br>";
              }
              logDiv.scrollTop = logDiv.scrollHeight;
            };

            ws.onerror = (error) => {
              console.error("WebSocket: Erro!", error);
            };

            ws.onclose = (event) => {
              console.log("WebSocket: Desconectado.", event.reason);
              if (!event.wasClean) {
                console.warning('Conexão perdida. Tentando reconectar...');
                setTimeout(connectWebSocket, 3000);
              } else {
                console.log('Conexão fechada.');
              }
            };
          }

          function iniciarGravacao() {
            console.log("Enviando comando para /gravar");
            fetch('/gravar')
              .then(response => {
                if (!response.ok) throw new Error('Falha ao iniciar gravacao');
                console.log("Comando /gravar enviado com sucesso.");
              })
              .catch(error => {
                console.error('Erro ao chamar /gravar:', error);
              });
          }

          function liberarManualmente() {
            console.log("Enviando comando para /liberar");
            fetch('/liberar')
              .then(response => {
                if (!response.ok) throw new Error('Falha ao liberar');
                console.log("Comando /liberar enviado com sucesso.");
              })
              .catch(error => {
                console.error('Erro ao chamar /liberar:', error);
              });
          }

          document.addEventListener('DOMContentLoaded', connectWebSocket);
        </script>
      </div>
    </body>
    </html>)====";
    server.send(200, "text/html", html);
  });

  server.on("/gravar", [&server]() {
    estadoGravacao = AGUARDANDO_NOVO_CARTAO;
    String latestLogEntry = "Modo de gravação ativado. <strong>Aproxime o cartão</strong> que deseja cadastrar.<br>";
    logEventos += latestLogEntry;
    webSocket.broadcastTXT("new_log:" + latestLogEntry);
    Serial.println("RFID: " + latestLogEntry);
    server.send(200, "text/plain", "OK");
  });

  server.on("/liberar", [&server]() {
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    delay(1000);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    String latestLogEntry = "Acesso <strong class=\"success\">LIBERADO</strong> remotamente<br>";
    logEventos += latestLogEntry;
    webSocket.broadcastTXT("new_log:" + latestLogEntry);
    server.send(200, "text/plain", "OK");
  });
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("WebSocket: Cliente [%u] Desconectado!\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("WebSocket: Cliente [%u] Conectado de %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      webSocket.sendTXT(num, "full_log:" + logEventos);
      Serial.println("WebSocket: Log completo enviado para o novo cliente.");
    }
      break;
    case WStype_TEXT:
      Serial.printf("WebSocket: Cliente [%u] recebeu Texto: %s\n", num, payload);
      break;
    default:
      break;
  }
}
