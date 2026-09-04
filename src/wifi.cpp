#include <wifi.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>


String AP_SSID = "ESP";
String AP_PASSWD = "";
String STA_SSID = "";
String STA_PASSWD = "";

void wifi_on()
{
    WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(AP_SSID.c_str(), AP_PASSWD.c_str());
    if(STA_SSID.length() > 0)
    {
        WiFi.begin(STA_SSID.c_str(), STA_PASSWD.c_str());
    }
}

void wifi_off()
{
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void wifi_toggle_onoff()
{
    WiFiMode_t CurrentMode = WiFi.getMode();
    if(CurrentMode == WIFI_OFF)
    {
        wifi_on();
    }
    else
    {
        wifi_off();
    }
}

void wifi_toggle_mode()
{
    static bool toggleMode = false;

    if(toggleMode == false)
    {
        WiFi.mode(WIFI_AP);
	    WiFi.softAP(AP_SSID.c_str(), AP_PASSWD.c_str());
        toggleMode = true;
    }
    else
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(STA_SSID.c_str(), STA_PASSWD.c_str());
        toggleMode = false;
    }

}