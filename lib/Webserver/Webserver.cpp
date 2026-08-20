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
}

void WebserverSetup()
{
	Serial.begin(115200);
	Serial.println();

	WiFi.mode(WIFI_AP);
	WiFi.softAP(SSID, PASSWD);
	server.begin();
	delay(100);
	WebserverConnectAPIs();
	Serial.printf("Server Running");
}

void WebserverHandleClients()
{
	server.handleClient();
	ProcessSchedules();
}
