#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPI.h>

#include "Estado.h"
#include "RFIDController.h"
#include "WebInterface.h"

const char* ssid = "SSID";
const char* password = "SENHA";

WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  
  Serial.println();
  Serial.print("Conectando a rede: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado com sucesso!");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());

  SPI.begin();

  initRFIDController();

  initWebInterface(server);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Servidor WebSocket iniciado na porta 81.");

  server.begin();
  Serial.println("Servidor HTTP iniciado na porta 80.");
}

void loop() {
  webSocket.loop();

  server.handleClient();

  verificarCartao();
}
