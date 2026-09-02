#include "Arduino.h"

#include <ESP8266WiFi.h>

#include <Webserver.h>
#include <api_handlers.h>
#include <wifi.h>
#include <log.h>

ESP8266WebServer server(80);

void WebserverConnectAPIs()
{
	server.on("/", HTTP_GET, handle_Root);
	server.on("/api/stats", HTTP_GET, handle_GetStatus);
	server.on("/api/timer/set", HTTP_PUT, handle_SetTimer);
	server.on("/api/date/set", HTTP_PUT, handle_SetSysTimestamp);
	server.on("/api/date/get", HTTP_GET, handle_GetDate);
	server.on("/api/schedules/get", HTTP_GET, handle_GetSchedules);
	server.on("/api/schedules/set", HTTP_PUT, handle_SetSchedule);
	server.on("/api/schedules/reset", HTTP_PUT, handle_ResetSchedules);
	server.on("/api/schedules/remove", HTTP_PUT, handle_RemoveSchedule);
	server.on("/api/wifi/setting", HTTP_PUT, handle_wifisetting);
	server.on("/api/wifi/toggle", HTTP_PUT, handle_wifitoggle);
	server.on("/api/log/download", HTTP_GET, handle_GetLog);
	server.on("/api/log/remove", HTTP_DELETE, handle_RemoveLog);
	server.onNotFound([]() {
		server.send(404, "text/html", "<h1>404 Not Found</h1>");
	});
}

void WebserverSetup()
{
	wifi_on();
	server.begin();
	delay(100);
	WebserverConnectAPIs();
	Serial.println("Server Running");
	Log("Webserver Running");
}

void WebserverHandleClients()
{
	server.handleClient();
}
