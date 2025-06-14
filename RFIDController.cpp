#include "RFIDController.h"
#include "WebInterface.h"
#include "Estado.h"
#include <SPI.h>

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
std::vector<String> uidsAutorizados;
unsigned long ultimaVerificacao = 0;
const unsigned long intervalo = 200;

void initRFIDController() {
  Serial.println("RFID: Inicializando controlador...");
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println("RFID: Leitor MFRC522 inicializado.");
}

String getUID() {
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

void verificarCartao() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String uidStr = getUID();
  String latestLogEntry;

  if (estadoGravacao == AGUARDANDO_NOVO_CARTAO) {
    digitalWrite(LED_VERMELHO, LOW);
    bool uidJaExiste = false;
    for (const String& uid : uidsAutorizados) {
      if (uid == uidStr) {
        uidJaExiste = true;
        break;
      }
    }

    if (!uidJaExiste) {
      uidsAutorizados.push_back(uidStr);
      latestLogEntry = "Cartão cadastrado <strong class=\"success\">com sucesso</strong> | UID: " + uidStr + "<br>";
      digitalWrite(LED_VERDE, HIGH);
      delay(1000);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_VERMELHO, HIGH);
    } else {
      latestLogEntry = "Cartão " + uidStr + "<strong> já cadastrado</strong>. Saindo do modo gravação...<br>";
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_VERMELHO, HIGH);
        delay(200);
        digitalWrite(LED_VERMELHO, LOW);
        delay(200);
        digitalWrite(LED_VERMELHO, HIGH);
      }
    }
    
    estadoGravacao = NENHUM; 
    Serial.println("RFID: Gravação concluída. Retornando ao modo normal.");
  } 
  else {
    bool autorizado = false;
    for (const String& uid : uidsAutorizados) {
      if (uid == uidStr) {
        autorizado = true;
        break;
      }
    }

    if (autorizado) {
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, HIGH);
      delay(1000);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_VERMELHO, HIGH);
      latestLogEntry = "Acesso <strong class=\"success\">LIBERADO</strong> | UID: " + uidStr + "<br>";
    } else {
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_VERMELHO, HIGH);
        delay(200);
        digitalWrite(LED_VERMELHO, LOW);
        delay(200);
        digitalWrite(LED_VERMELHO, HIGH);
      }
      latestLogEntry = "Acesso <strong class=\"error\">NEGADO</strong>. UID não reconhecido. | UID: " + uidStr + "<br>";
    }
  }

  logEventos += latestLogEntry;
  webSocket.broadcastTXT("new_log:" + latestLogEntry);
  Serial.println("RFID: " + latestLogEntry);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
