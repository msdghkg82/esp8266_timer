#ifndef WEBSERVER_H_
#define WEBSERVER_H_


#pragma once

#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

void WebserverSetup();

void WebserverHandleClients();

void OTA_Setup(ESP8266WebServer &server);

#endif /* WEBSERVER_H_ */