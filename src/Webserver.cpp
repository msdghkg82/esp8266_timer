#include "Arduino.h"

#include <ESP8266WiFi.h>

#include <Webserver.h>
#include <api_handlers.h>

const char * SSID = "ESP";
const char * PASSWD = "12345678";

ESP8266WebServer server(80);

void WebserverConnectAPIs()
{
	server.on("/", HTTP_GET, handleRoot);
	server.on("/api/stats", HTTP_GET, handleStatus);
	server.on("/api/timer", HTTP_PUT, handleTimer);
	server.on("/api/timestamp", HTTP_PUT, handleTimestamp);
	server.on("/api/date", HTTP_GET, handleDate);
	server.on("/api/setschedule", HTTP_PUT, handleSchedule);
    server.on("/api/getschedules", HTTP_GET, handleGetSchedules);
	server.on("/api/resetschedules", HTTP_GET, handleResetSchedules);
	server.on("/api/removeschedule", HTTP_PUT, handleRemoveSchedule);
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
