#include "Estado.h"

EstadoGravacao estadoGravacao = NENHUM;
String logEventos = "";

WebSocketsServer webSocket = WebSocketsServer(81);
