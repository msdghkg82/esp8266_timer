#include <wifi.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <savefile.h>


String SSID = "ESP";
String PASSWD = "12345678";

void wifi_setup()
{
    WiFi.mode(WIFI_AP);
	WiFi.softAP(SSID.c_str(), PASSWD.c_str());
}

void wifi_off()
{
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}