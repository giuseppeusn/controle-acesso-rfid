#pragma once
#include <MFRC522.h>
#include "Estado.h"

#define SS_PIN         5
#define RST_PIN        22
#define LED_VERDE      16
#define LED_VERMELHO   2

extern MFRC522 mfrc522;
void initRFIDController();
void verificarCartao();
String getUID();