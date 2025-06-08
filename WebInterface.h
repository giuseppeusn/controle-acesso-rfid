#pragma once
#include <WebServer.h>
#include <WebSocketsServer.h>


void initWebInterface(WebServer &server);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void handleListAuthorizedUIDs(WebServer& server);

void handleRemoveUID(WebServer& server);
