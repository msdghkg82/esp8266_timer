#include <button.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <scheduler.h>
#include <RTC.h>


#define BUTTON_PIN 20

volatile uint32_t pressStartTime = 0;
volatile uint32_t pressDuration = 0;
volatile bool buttonReleased = false;


void IRAM_ATTR buttonISR()
{
    if (digitalRead(BUTTON_PIN) == LOW)
    {
        // Falling edge
        pressStartTime = millis();
    }
    else
    {
        // Rising edge
        pressDuration = millis() - pressStartTime;
        buttonReleased = true;
    }
}


void ButtonInit()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonISR,
        CHANGE
    );
}


void ButtonTask()
{
    if(buttonReleased)
    {
        buttonReleased = false;

        if(pressDuration >= 20000)
        {
            Serial.println("20 second press");
            ResetSchedules();
            systemtimestamp = 0;
        }
        else if(pressDuration >= 10000)
        {
            Serial.println("10 second press");
            WiFi.mode(WIFI_OFF);
        }
        else
        {
            Serial.println("Short press");
        }
    }
}