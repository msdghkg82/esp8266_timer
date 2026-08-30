#include <wifi.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <savefile.h>

const char * SSID;// = "ESP";
const char * PASSWD;// = "12345678";

void wifi_setup()
{
    WiFi.mode(WIFI_AP);
	WiFi.softAP(SSID, PASSWD);
}