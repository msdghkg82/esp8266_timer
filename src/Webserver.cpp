#include "Arduino.h"

#include <ESP8266WiFi.h>

#include <Webserver.h>
#include <api_handlers.h>
#include <scheduler.h>

String SSID = "ESP";
String PASSWD = "12345678";

ESP8266WebServer server(80);

void WebserverConnectAPIs()
{
	server.on("/", HTTP_GET, handle_Root);
	server.on("/api/stats", HTTP_GET, handle_GetStatus);
	server.on("/api/timer", HTTP_PUT, handle_SetTimer);
	server.on("/api/settimestamp", HTTP_PUT, handle_SetSysTimestamp);
	server.on("/api/getdate", HTTP_GET, handle_GetDate);
	server.on("/api/setschedule", HTTP_PUT, handle_SetSchedule);
    server.on("/api/getschedules", HTTP_GET, handle_GetSchedules);
	server.on("/api/resetschedules", HTTP_GET, handle_ResetSchedules);
	server.on("/api/removeschedule", HTTP_PUT, handle_RemoveSchedule);
	server.on("/api/getsavedfile", HTTP_GET, handle_GetSavedFile);
}

void WebserverSetup()
{
	WiFi.mode(WIFI_AP);
	WiFi.softAP(SSID, PASSWD);
	server.begin();
	delay(100);
	WebserverConnectAPIs();
	Serial.println("Server Running");
}

void WebserverHandleClients()
{
	server.handleClient();
	ProcessSchedules();
}
