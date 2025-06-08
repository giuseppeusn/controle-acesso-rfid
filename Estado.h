#pragma once

#include <WebSocketsServer.h>

enum EstadoGravacao {
  NENHUM,
  AGUARDANDO_NOVO_CARTAO
};

extern EstadoGravacao estadoGravacao;
extern String logEventos;
extern WebSocketsServer webSocket;
